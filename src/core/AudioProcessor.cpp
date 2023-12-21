/**
 *
 *  @file      AudioProcessor.cpp
 *  @author    Hallvard Jensen
 *  @date      22 Jun 2021
 *  @copyright Hallvard Jensen, 2022. All right reserved.
 *
 */

#include "AudioProcessor.h"
#include "../util/Globals.h"

namespace {
double midiToFrequency(const int &note) {
    // Tuning is unchangeable due to the MIDI protocol.
    constexpr double tuning{440.0};
    constexpr double a4{69.0};
    constexpr double octave{12.0};

    // Based on MIDI tuning standard
    return std::pow(2, (note - a4) / octave) * tuning;
}
} // namespace

anyMidi::AudioProcessor::AudioProcessor(double sampleRate,
                                        const juce::ValueTree &v)
    : fft_{sampleRate, juce::dsp::WindowingFunction<float>::hamming},
      deviceManager_{new anyMidi::AudioDeviceManagerRCO()},
      midiProc_{static_cast<unsigned int>(sampleRate),
                juce::Time::getMillisecondCounterHiRes() * msToSec},
      tree_{v} {
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
        const juce::File deviceSettingsFile =
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

    // Generate a list of frequencies corresponding to the 128 Midi notes
    constexpr int midiUpperBound{140};
    for (int i = 0; i < midiUpperBound; ++i) {
        noteFrequencies_.push_back(midiToFrequency(i));
    }

    // Adding device manager to ValueTree so AudioDeviceSelectorComponent in GUI
    // can access it. No listeners needed since pointer to device manager is
    // received by GUI.
    tree_.getChildWithName(anyMidi::AUDIO_PROC_ID)
        .setProperty(anyMidi::DEVICE_MANAGER_ID, deviceManager_.getObject(),
                     nullptr);

    auto guiNode = tree_.getChildWithName(anyMidi::GUI_ID);
    guiNode.setProperty(anyMidi::ATTACK_THRESH_ID,
                        midiProc_.getAttackThreshold(), nullptr);
    guiNode.setProperty(anyMidi::RELEASE_THRESH_ID,
                        midiProc_.getReleaseThreshold(), nullptr);
    guiNode.setProperty(anyMidi::PARTIALS_ID, numPartials_, nullptr);
    guiNode.setProperty(anyMidi::LO_CUT_ID, lowFilterFreq, nullptr);
    guiNode.setProperty(anyMidi::HI_CUT_ID, highFilterFreq, nullptr);

    guiNode.setProperty(anyMidi::CURRENT_WIN_ID, fft_.getWindowingFunction(),
                        nullptr);

    juce::ValueTree winNode{anyMidi::ALL_WIN_ID};
    guiNode.addChild(winNode, -1, nullptr);

    auto win = fft_.getAvailableWindowingMethods();
    for (const auto &w : win) {
        juce::ValueTree winItemNode{anyMidi::WIN_NODE_ID};
        winNode.addChild(
            winItemNode.setProperty(anyMidi::WIN_NAME_ID, w, nullptr), -1,
            nullptr);
    }

    // Register this class as listener to ValueTree.
    tree_.addListener(this);
}

anyMidi::AudioProcessor::~AudioProcessor() {
    audioSourcePlayer_.setSource(nullptr);
    deviceManager_->removeAudioCallback(&audioSourcePlayer_);
    deviceManager_ = nullptr;
}

void anyMidi::AudioProcessor::prepareToPlay(int samplesPerBlockExpected,
                                            double sampleRate) {
    processingBuffer_.setSize(numInputChannels, samplesPerBlockExpected, false,
                              true);
    midiProc_.setMidiOutput(deviceManager_->getDefaultMidiOutput());

    // Initializing highpass filter.
    hiPassFilter_.setCoefficients(
        juce::IIRCoefficients::makeHighPass(sampleRate, lowFilterFreq));
    hiPassFilter_.reset();
}

void anyMidi::AudioProcessor::getNextAudioBlock(
    const juce::AudioSourceChannelInfo &bufferToFill) {
    if (bufferToFill.buffer->getNumChannels() > 0) {
        const int numSamples =
            bufferToFill.buffer->getNumSamples(); // To make sure copied buffer
                                                  // size is the same.

        // Copies actual buffer into a buffer for processing.
        for (int i = 0; i < processingBuffer_.getNumChannels(); ++i) {
            processingBuffer_.copyFrom(
                i, 0, bufferToFill.buffer->getReadPointer(i), numSamples);
        }

        const auto *channelData =
            processingBuffer_.getReadPointer(0, bufferToFill.startSample);
        auto *outBuffer =
            processingBuffer_.getWritePointer(0, bufferToFill.startSample);

        // Puts samples into non-const buffer.
        for (int i = 0; i < numSamples; ++i) {
            outBuffer[i] = channelData[i];
        }

        // Applies filter.
        hiPassFilter_.processSamples(outBuffer, numSamples);

        // Puts samples into FFT fifo after processing.
        for (int i = 0; i < numSamples; ++i) {
            fft_.pushNextSampleIntoFifo(outBuffer[i]);
        }
    }

    if (fft_.isNextFFTBlockReady()) {
        calcNote();
        fft_.setNextFFTBlockReady(false);
    }

    midiProc_.pushBufferToOutput();
}

void anyMidi::AudioProcessor::releaseResources() {
    midiProc_.turnOffAllMessages();
}

void anyMidi::AudioProcessor::calcNote() {
    auto noteInfo = analyzeHarmonics(); // Gets {note, amplitude}
    const int note = noteInfo.first;
    const double amp = noteInfo.second;
    const int velocity = static_cast<int>(std::round(amp * 127));

    // auto noteInfo = fft.calcFundamentalFreq();
    // int note = findNearestNote(noteInfo.first);

    std::vector<std::pair<int, bool>> noteValues;
    if (midiProc_.determineNoteValue(note, amp, noteValues)) {
        for (const auto &newNote : noteValues) {
            if (newNote.second) {
                midiProc_.createMidiMsg(newNote.first,
                                        static_cast<juce::uint8>(velocity),
                                        newNote.second);
            } else {
                midiProc_.createMidiMsg(newNote.first, 0, newNote.second);
            }
        }
    }
}

std::pair<int, double> anyMidi::AudioProcessor::analyzeHarmonics() {
    auto harmonics = fft_.getHarmonics(numPartials_, noteFrequencies_);

    std::map<int, double> scores;
    double totalAmp{0.0};

    for (int i = 0; i < numPartials_; ++i) {
        const double freq = noteFrequencies_[harmonics[i].first];

        // Calculates fundamental frequency of partial based on index.
        const double fundamental = freq / (i + 1);

        // Get nearest MIDI note value of frequency.
        auto note = anyMidi::findNearestNote(fundamental, noteFrequencies_);

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
    for (const auto &score : scores) {
        if (score.second > maxScore) {
            correctNote = score.first;
            maxScore = score.second;
        }
    }

    auto analyzedNote = std::make_pair(correctNote, totalAmp);
    return analyzedNote;
}

void anyMidi::AudioProcessor::setAudioChannels(
    int numInputChannels, int numOutputChannels,
    const juce::XmlElement *const storedSettings) {
    const juce::String audioError = deviceManager_->initialise(
        numInputChannels, numOutputChannels, storedSettings, true);

    if (audioError.isEmpty()) {
        anyMidi::log(tree_, audioError);
    }

    deviceManager_->addAudioCallback(&audioSourcePlayer_);
    audioSourcePlayer_.setSource(this);
}

void anyMidi::AudioProcessor::valueTreePropertyChanged(
    juce::ValueTree &treeWhosePropertyHasChanged,
    const juce::Identifier &property) {
    if (property == anyMidi::ATTACK_THRESH_ID) {
        double t = treeWhosePropertyHasChanged.getProperty(property);
        midiProc_.setAttackThreshold(t);
    } else if (property == anyMidi::RELEASE_THRESH_ID) {
        double t = treeWhosePropertyHasChanged.getProperty(property);
        midiProc_.setReleaseThreshold(t);
    } else if (property == anyMidi::PARTIALS_ID) {
        int n = treeWhosePropertyHasChanged.getProperty(property);
        setNumPartials(n);
    } else if (property == anyMidi::LO_CUT_ID) {
        const double f = treeWhosePropertyHasChanged.getProperty(property);
        hiPassFilter_.setCoefficients(juce::IIRCoefficients::makeHighPass(
            deviceManager_.get()->getAudioDeviceSetup().sampleRate, f));
    } else if (property == anyMidi::CURRENT_WIN_ID) {
        const int w = treeWhosePropertyHasChanged.getProperty(property);
        fft_.setWindowingFunction(w);
    }
}

void anyMidi::AudioProcessor::setNumPartials(int &n) { numPartials_ = n; }