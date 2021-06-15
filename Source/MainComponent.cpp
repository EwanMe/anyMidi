#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() :
    fft{ 48000 },
    midiProc{ 48000, juce::Time::getMillisecondCounterHiRes() * 0.001 },
    tabs{ deviceManager }//,
    //audioSetupComp{
    //    deviceManager,
    //    0,      // min input ch
    //    256,    // max input ch
    //    0,      // min output ch
    //    256,    // max output ch
    //    false,  // can select midi inputs?
    //    true,   // can select midi output device?
    //    false,  // treat channels as stereo pairs
    //    false } // hide advanced options?
{
    // Some platforms require permissions to open input channels so request that here.
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        juce::File deviceSettingsFile = juce::File::getCurrentWorkingDirectory().getChildFile("audio_device_settings.xml");
        
        if (deviceSettingsFile.existsAsFile())
        {
            // Loads settings from file if it exists.
            const auto storedSettings = juce::parseXML(deviceSettingsFile);
            setAudioChannels(numInputChannels, numOutputChannels, storedSettings.get());
        }
        else
        {
            setAudioChannels(numInputChannels, numOutputChannels);
        }
    }
    
    // Audio device manager.
    //addAndMakeVisible(audioSetupComp);
    addAndMakeVisible(tabs);

    //// Gain slider.
    //addAndMakeVisible(gainSlider);
    //gainSlider.setRange(0, 1, 0.01);
    //gainSlider.setValue(0.0);

    //// Output box, used for debugging.
    //addAndMakeVisible(outputBox);
    //outputBox.setMultiLine(true);
    //outputBox.setReturnKeyStartsNewLine(true);
    //outputBox.setReadOnly(true);
    //outputBox.setScrollbarsShown(true);
    //outputBox.setCaretVisible(false);
    //outputBox.setPopupMenuEnabled(true);
    //outputBox.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x32ffffff));
    //outputBox.setColour(juce::TextEditor::outlineColourId, juce::Colour(0x1c000000));
    //outputBox.setColour(juce::TextEditor::shadowColourId, juce::Colour(0x16000000));

    //addAndMakeVisible(clearOutput);
    //clearOutput.setButtonText("Clear output");
    //clearOutput.onClick = [this]
    //{
    //    outputBox.clear();
    //};

    setSize(500, 500);

    // Generates a list of frequencies corresponding to the 128 Midi notes
    // based on the global tuning.
    // Thanks to http://subsynth.sourceforge.net/midinote2freq.html for this snippet.
    for (int i = 0; i < 140; ++i)
    {
        noteFrequencies.push_back((tuning / 32.0) * std::pow(2, (i - 9.0) / 12.0));
    }
}

MainComponent::~MainComponent()
{   
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    processingBuffer.setSize(numInputChannels, samplesPerBlockExpected, false, true);
    midiProc.setMidiOutput(deviceManager.getDefaultMidiOutput());

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlockExpected;
    spec.numChannels = numOutputChannels;

    noiseGate.prepare(spec);
    noiseGate.setThreshold(-1.0); //50
    noiseGate.setAttack(5.0);
    noiseGate.setRelease(17.0);
    noiseGate.setRatio(1.0);

    // Initializing highpass filter.
    hiPassFilter.setCoefficients(juce::IIRCoefficients::makeHighPass(sampleRate, 75.0, 1.0)); // E5 on guitar = ~82 Hz
    hiPassFilter.reset();
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (bufferToFill.buffer->getNumChannels() > 0)
    {
        int numSamples = bufferToFill.buffer->getNumSamples(); // To make sure copied buffer size is the same.

        // Copies actual buffer into a buffer for processing.
        for (int i = 0; i < processingBuffer.getNumChannels(); ++i)
        {
            processingBuffer.copyFrom(i, 0, bufferToFill.buffer->getReadPointer(i), numSamples);
        }

        juce::dsp::AudioBlock<float> block{ processingBuffer };
        juce::dsp::ProcessContextReplacing<float> context{ block };
        //noiseGate.process(context);

        auto* channelData = processingBuffer.getReadPointer(0, bufferToFill.startSample);
        auto* outBuffer = processingBuffer.getWritePointer(0, bufferToFill.startSample);
        
        // Puts samples into non-const buffer.
        for (unsigned int i = 0; i < numSamples; ++i)
        {
            outBuffer[i] = channelData[i];
        }

        // Applies filter.
        hiPassFilter.processSamples(outBuffer, numSamples);

        // Puts samples into FFT fifo after processing.
        for (unsigned int i = 0; i < numSamples; ++i)
        {
            fft.pushNextSampleIntoFifo(outBuffer[i]);
        }

        // Just for audio output. Can be removed --
        for (int i = 0; i < processingBuffer.getNumChannels(); ++i)
        {
            bufferToFill.buffer->copyFrom(i, 0, processingBuffer, i, 0, numSamples);
        }

        auto* read = bufferToFill.buffer->getReadPointer(0, bufferToFill.startSample);
        auto* write_l = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
        auto* write_r = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);
        for (unsigned int i = 0; i < numSamples; ++i)
        {
            write_l[i] = read[i] * gainSlider.getValue();
            write_r[i] = read[i] * gainSlider.getValue();
        }
        // -- can be removed
    }

    if (fft.nextFFTBlockReady)
    {       
        calcNote();
        fft.nextFFTBlockReady = false;
    }

    midiProc.pushBufferToOutput();
}

void MainComponent::releaseResources()
{
    //auto audioDeviceSettings = audioSetupComp.deviceManager.createStateXml();

    //if (audioDeviceSettings != nullptr)
    //{
    //    // Writes user settings to XML file for storage.
    //    juce::File settingsFileName = juce::File::getCurrentWorkingDirectory().getChildFile("audio_device_settings.xml");
    //    settingsFileName.replaceWithText(audioDeviceSettings->toString());
    //}

    midiProc.turnOffAllMessages();
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    auto rect = getLocalBounds();
    
    auto halfWidth = getWidth() / 2;
    // auto halfHeight = getHeight() / 2;
    
    // audioSetupComp.setBounds(rect.withWidth(getWidth()));
    tabs.setBounds(rect.reduced(4));

    // clearOutput.setBounds(rect.getCentreX(), 15, halfWidth / 2 - 10, 20);

    // gainSlider.setBounds(rect.getCentreX(), 45, halfWidth / 2 - 10, 20);

    // outputBox.setBounds(halfWidth, 85, halfWidth - 10, halfHeight + 40);
}

void MainComponent::calcNote()
{
    auto noteInfo = analyzeHarmonics(); // Gets {note, amplitude}
    int note = noteInfo.first;
    // auto noteInfo = fft.calcFundamentalFreq();
    // int note = findNearestNote(noteInfo.first);
    double amp = noteInfo.second;
    int velocity = (int)std::round(amp * 127);

    std::vector<std::pair<int, bool>> noteValues;
    if (midiProc.determineNoteValue(note, amp, noteValues))
    {
        for (std::pair newNote : noteValues)
        {
            if (newNote.second == true)
            {
                midiProc.createMidiMsg(newNote.first, velocity, newNote.second);
            }
            else
            {
                // Problems with turning correct midi note off led to just turning
                // everything off. This works for monophonic playing.
                midiProc.createMidiMsg(newNote.first, 0, newNote.second);
            }
        }
    }
}

std::pair<int, double> MainComponent::analyzeHarmonics()
{
    constexpr int numHarm{ 6 };
    auto harmonics = fft.getHarmonics(numHarm, noteFrequencies);

    int fundamental{ 0 };
    double maxAmp{ 0.0 };
    std::map<int, double> scores;
    double totalAmp{ 0.0 };
    for (int i = 0; i < numHarm; ++i)
    {
        double freq = noteFrequencies[harmonics[i].first];
        
        // Calculates fundamental frequency of partial based on index.
        double fundamental = freq / (i + 1);
        
        // Get nearest MIDI note value of frequency.
        int note = anyMidi::findNearestNote(fundamental, noteFrequencies);
        
        // Scoring weighted based on log2 of freq. FFT bins are distributed linearly and freqencies are percieved
        // logarithmically. This aims to let the high frequency bins of the FFT with high resolution have more weighting on score.
        scores[note] += 1.0 * log2(fundamental);

        // Amps of partials added together to represent true amplitude.
        totalAmp += harmonics[i].second;
    }

    // Finds note with highest score.
    int correctNote{ 0 };
    double maxScore{ 0.0 };
    for (std::pair<int, double> s : scores)
    {
        if (s.second > maxScore)
        {
            correctNote = s.first;
            maxScore = s.second;
        }
    }

    std::pair<int, double> analyzedNote = std::make_pair(correctNote, totalAmp);
    return analyzedNote;
}

void MainComponent::log(const juce::MidiMessage& midiMessage)
{
    outputBox.moveCaretToEnd();
    outputBox.insertTextAtCaret(midiMessage.getDescription() + juce::newLine);
}

template<typename T>
void MainComponent::log(T msg)
{
    outputBox.moveCaretToEnd();
    outputBox.insertTextAtCaret(std::to_string(msg) + juce::newLine);
}