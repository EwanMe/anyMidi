/**
 *
 *  @file      AudioProcessor.cpp
 *  @author    Hallvard Jensen
 *  @date      22 Jun 2021
 *  @copyright Hallvard Jensen, 2022. All right reserved.
 *
 */

#include "AudioProcessor.h"

#include <JuceHeader.h>

#include "../util/Globals.h"

using namespace anyMidi;

AudioProcessor::AudioProcessor(juce::ValueTree v)
    : fft{48000, juce::dsp::WindowingFunction<float>::hamming},
      midiProc{48000, juce::Time::getMillisecondCounterHiRes() * 0.001},
      tree{v} {
    deviceManager = new anyMidi::AudioDeviceManagerRCO();

    // Some platforms require permissions to open input channels so requesting
    // this here.
    if (juce::RuntimePermissions::isRequired(
            juce::RuntimePermissions::recordAudio) &&
        !juce::RuntimePermissions::isGranted(
            juce::RuntimePermissions::recordAudio)) {
        juce::RuntimePermissions::request(
            juce::RuntimePermissions::recordAudio, [&](bool granted) {
                setAudioChannels(granted ? numInputChannels : 0,
                                 numOutputChannels);
            });
    } else {
        juce::File deviceSettingsFile =
            juce::File::getCurrentWorkingDirectory().getChildFile(
                anyMidi::AUDIO_SETTINGS_FILENAME);

        if (deviceSettingsFile.existsAsFile()) {
            // Loads settings from file if it exists.
            const auto storedSettings = juce::parseXML(deviceSettingsFile);
            setAudioChannels(numInputChannels, numOutputChannels,
                             storedSettings.get());
        } else {
            setAudioChannels(numInputChannels, numOutputChannels);
        }
    }

    // Generates a list of frequencies corresponding to the 128 Midi notes
    // based on the global tuning.
    // Thanks to http://subsynth.sourceforge.net/midinote2freq.html for this
    // snippet.
    for (int i = 0; i < 140; ++i) {
        noteFrequencies.push_back((tuning / 32.0) *
                                  std::pow(2, (i - 9.0) / 12.0));
    }

    // Adding device manager to ValueTree so AudioDeviceSelectorComponent in GUI
    // can access it. No listeners needed since pointer to device manager is
    // received by GUI.
    tree.getChildWithName(anyMidi::AUDIO_PROC_ID)
        .setProperty(anyMidi::DEVICE_MANAGER_ID, deviceManager.getObject(),
                     nullptr);

    auto guiNode = tree.getChildWithName(anyMidi::GUI_ID);
    guiNode.setProperty(anyMidi::ATTACK_THRESH_ID,
                        midiProc.getAttackThreshold(), nullptr);
    guiNode.setProperty(anyMidi::RELEASE_THRESH_ID,
                        midiProc.getReleaseThreshold(), nullptr);
    guiNode.setProperty(anyMidi::PARTIALS_ID, numPartials, nullptr);
    guiNode.setProperty(anyMidi::LO_CUT_ID, lowFilterFreq, nullptr);
    guiNode.setProperty(anyMidi::HI_CUT_ID, highFilterFreq, nullptr);

    guiNode.setProperty(anyMidi::CURRENT_WIN_ID, fft.getWindowingFunction(),
                        nullptr);

    juce::ValueTree winNode{anyMidi::ALL_WIN_ID};
    guiNode.addChild(winNode, -1, nullptr);

    auto win = fft.getAvailableWindowingMethods();
    for (juce::String w : win) {
        juce::ValueTree winItemNode{anyMidi::WIN_NODE_ID};
        winNode.addChild(
            winItemNode.setProperty(anyMidi::WIN_NAME_ID, w, nullptr), -1,
            nullptr);
    }

    // Register this class as listener to ValueTree.
    tree.addListener(this);
}

AudioProcessor::~AudioProcessor() {
    audioSourcePlayer.setSource(nullptr);
    deviceManager->removeAudioCallback(&audioSourcePlayer);
    deviceManager = nullptr;

    // Maybe superfluous?
    jassert(audioSourcePlayer.getCurrentSource() == nullptr);
}

void AudioProcessor::prepareToPlay(int samplesPerBlockExpected,
                                   double sampleRate) {
    processingBuffer.setSize(numInputChannels, samplesPerBlockExpected, false,
                             true);
    midiProc.setMidiOutput(deviceManager->getDefaultMidiOutput());

    // Initializing highpass filter.
    hiPassFilter.setCoefficients(
        juce::IIRCoefficients::makeHighPass(sampleRate, lowFilterFreq));
    hiPassFilter.reset();
}

void AudioProcessor::getNextAudioBlock(
    const juce::AudioSourceChannelInfo &bufferToFill) {
    if (bufferToFill.buffer->getNumChannels() > 0) {
        int numSamples =
            bufferToFill.buffer->getNumSamples(); // To make sure copied buffer
                                                  // size is the same.

        // Copies actual buffer into a buffer for processing.
        for (int i = 0; i < processingBuffer.getNumChannels(); ++i) {
            processingBuffer.copyFrom(
                i, 0, bufferToFill.buffer->getReadPointer(i), numSamples);
        }

        auto *channelData =
            processingBuffer.getReadPointer(0, bufferToFill.startSample);
        auto *outBuffer =
            processingBuffer.getWritePointer(0, bufferToFill.startSample);

        // Puts samples into non-const buffer.
        for (int i = 0; i < numSamples; ++i) {
            outBuffer[i] = channelData[i];
        }

        // Applies filter.
        hiPassFilter.processSamples(outBuffer, numSamples);

        // Puts samples into FFT fifo after processing.
        for (int i = 0; i < numSamples; ++i) {
            fft.pushNextSampleIntoFifo(outBuffer[i]);
        }
    }

    if (fft.nextFFTBlockReady) {
        calcNote();
        fft.nextFFTBlockReady = false;
    }

    midiProc.pushBufferToOutput();
}

void AudioProcessor::releaseResources() { midiProc.turnOffAllMessages(); }

void AudioProcessor::calcNote() {
    auto noteInfo = analyzeHarmonics(); // Gets {note, amplitude}
    int note = noteInfo.first;
    double amp = noteInfo.second;
    int velocity = (int)std::round(amp * 127);

    // auto noteInfo = fft.calcFundamentalFreq();
    // int note = findNearestNote(noteInfo.first);

    std::vector<std::pair<int, bool>> noteValues;
    if (midiProc.determineNoteValue(note, amp, noteValues)) {
        for (std::pair newNote : noteValues) {
            if (newNote.second == true) {
                midiProc.createMidiMsg(newNote.first, velocity, newNote.second);
            } else {
                midiProc.createMidiMsg(newNote.first, 0, newNote.second);
            }
        }
    }
}

std::pair<int, double> AudioProcessor::analyzeHarmonics() {
    auto harmonics = fft.getHarmonics(numPartials, noteFrequencies);

    std::map<int, double> scores;
    double totalAmp{0.0};

    for (int i = 0; i < numPartials; ++i) {
        double freq = noteFrequencies[harmonics[i].first];

        // Calculates fundamental frequency of partial based on index.
        double fundamental = freq / (i + 1);

        // Get nearest MIDI note value of frequency.
        int note = anyMidi::findNearestNote(fundamental, noteFrequencies);

        // Scoring weighted based on log2 of freq. FFT bins are distributed
        // linearly and freqencies are percieved logarithmically. This aims to
        // let the high frequency bins of the FFT with high resolution have more
        // weighting on score.
        scores[note] += 1.0 * log2(fundamental);

        // Amps of partials added together to represent true amplitude.
        totalAmp += harmonics[i].second;
    }

    int correctNote{0};
    double maxScore{0.0};

    // Finds note with highest score.
    for (std::pair<int, double> s : scores) {
        if (s.second > maxScore) {
            correctNote = s.first;
            maxScore = s.second;
        }
    }

    std::pair<int, double> analyzedNote = std::make_pair(correctNote, totalAmp);
    return analyzedNote;
}

void AudioProcessor::setAudioChannels(int numInputChannels,
                                      int numOutputChannels,
                                      const juce::XmlElement *const xml) {
    juce::String audioError = deviceManager->initialise(
        numInputChannels, numOutputChannels, xml, true);

    // jassert(audioError.isEmpty());
    if (audioError.isEmpty()) {
        anyMidi::log(tree, audioError);
    }

    deviceManager->addAudioCallback(&audioSourcePlayer);
    audioSourcePlayer.setSource(this);
}

void AudioProcessor::valueTreePropertyChanged(
    juce::ValueTree &treeWhosePropertyHasChanged,
    const juce::Identifier &property) {
    if (property == anyMidi::ATTACK_THRESH_ID) {
        double t = treeWhosePropertyHasChanged.getProperty(property);
        midiProc.setAttackThreshold(t);
    } else if (property == anyMidi::RELEASE_THRESH_ID) {
        double t = treeWhosePropertyHasChanged.getProperty(property);
        midiProc.setReleaseThreshold(t);
    } else if (property == anyMidi::PARTIALS_ID) {
        int n = treeWhosePropertyHasChanged.getProperty(property);
        setNumPartials(n);
    } else if (property == anyMidi::LO_CUT_ID) {
        double f = treeWhosePropertyHasChanged.getProperty(property);
        hiPassFilter.setCoefficients(juce::IIRCoefficients::makeHighPass(
            deviceManager.get()->getAudioDeviceSetup().sampleRate, f));
    } else if (property == anyMidi::CURRENT_WIN_ID) {
        int w = treeWhosePropertyHasChanged.getProperty(property);
        fft.setWindowingFunction(w);
    }
}

void AudioProcessor::setNumPartials(int &n) { numPartials = n; }