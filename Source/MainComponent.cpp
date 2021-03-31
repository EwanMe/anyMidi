#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() :
    startTime{ juce::Time::getMillisecondCounterHiRes() * 0.001 },
    fft{ 48000 },
    midiOut{ nullptr },
    spectrogramImage{ juce::Image::RGB, 512, 512, true },
    audioSetupComp{
        deviceManager,
        0,      // min input ch
        256,    // max input ch
        0,      // min output ch
        256,    // max output ch
        false,  // can select midi inputs?
        true,   // can select midi output device?
        false,  // treat channels as stereo pairs
        false } // hide advanced options?
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
    addAndMakeVisible(audioSetupComp);

    // Gain slider.
    addAndMakeVisible(gainSlider);
    gainSlider.setRange(0, 1, 0.01);
    gainSlider.setValue(0.0);

    // Output box, used for debugging.
    addAndMakeVisible(outputBox);
    outputBox.setMultiLine(true);
    outputBox.setReturnKeyStartsNewLine(true);
    outputBox.setReadOnly(true);
    outputBox.setScrollbarsShown(true);
    outputBox.setCaretVisible(false);
    outputBox.setPopupMenuEnabled(true);
    outputBox.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x32ffffff));
    outputBox.setColour(juce::TextEditor::outlineColourId, juce::Colour(0x1c000000));
    outputBox.setColour(juce::TextEditor::shadowColourId, juce::Colour(0x16000000));

    addAndMakeVisible(clearOutput);
    clearOutput.setButtonText("Clear output");
    clearOutput.onClick = [this]
    {
        outputBox.clear();
    };

    // Timer used for messages.
    startTimer(500);

    setSize(1000, 600);

    // Generates a list of frequencies corresponding to the 128 Midi notes
    // based on the global tuning.
    // Thanks to http://subsynth.sourceforge.net/midinote2freq.html for this snippet.
    for (int i = 0; i < 128; ++i)
    {
        noteFrequencies[i] = (tuning / 32.0) * std::pow(2, (i - 9.0) / 12.0);
    }
}

MainComponent::~MainComponent()
{   
    auto audioDeviceSettings = audioSetupComp.deviceManager.createStateXml();

    if (audioDeviceSettings != nullptr)
    {
        // Writes user settings to XML file for storage.
        juce::File settingsFileName = juce::File::getCurrentWorkingDirectory().getChildFile("audio_device_settings.xml");
        settingsFileName.replaceWithText(audioDeviceSettings->toString());
    }

    midiOut->sendMessageNow(juce::MidiMessage::allNotesOff(midiChannel));
    
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // Initializing highpass filter.
    filter.setCoefficients(juce::IIRCoefficients::makeHighPass(sampleRate, 50.0, 1.0));
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (bufferToFill.buffer->getNumChannels() > 0)
    {
        auto* channelData = bufferToFill.buffer->getReadPointer(0, bufferToFill.startSample);
        auto* outBuffer_1 = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
        auto* outBuffer_2 = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);

        // float* input = new float { 0.0 };
        // filter.processSamples(input, bufferToFill.numSamples);
        for (unsigned int i = 0; i < bufferToFill.numSamples; ++i)
        {
            fft.pushNextSampleIntoFifo(channelData[i]);
            
            // Throughput for debugging purposes. Just sends singnal through, scaled by the gain slider.
            outBuffer_1[i] = channelData[i] * gainSlider.getValue();
            outBuffer_2[i] = channelData[i] * gainSlider.getValue();
        }
    }

    if (fft.nextFFTBlockReady)
    {
        // Draws the spectrogram image.
        // drawNextLineOfSpectrogram();
        
        calcNote();
        fft.nextFFTBlockReady = false;
        // repaint();
    }

    midiOut = deviceManager.getDefaultMidiOutput();
    if (midiOut)
    {
        midiOut->startBackgroundThread();
    }

    if (midiOut != nullptr)
    {
        midiOut->sendBlockOfMessagesNow(midiBuffer);
        midiBuffer.clear();
    }
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

void MainComponent::timerCallback()
{
    /*if (messagesPending.test_and_set())
    {
        drawNextLineOfSpectrogram();

        for (auto m : midiBuffer)
        {
            log(m.getMessage());
        }

        messagesPending.clear();
    }*/
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // Draws spectrogram underneath GUI. It works for now.
    g.drawImage(spectrogramImage, getLocalBounds().toFloat());
}

void MainComponent::resized()
{
    auto rect = getLocalBounds();
    
    auto halfWidth = getWidth() / 2;
    auto halfHeight = getHeight() / 2;

    audioSetupComp.setBounds(rect.withWidth(halfWidth));

    clearOutput.setBounds(rect.getCentreX(), 15, halfWidth / 2 - 10, 20);

    gainSlider.setBounds(rect.getCentreX(), 45, halfWidth / 2 - 10, 20);

    outputBox.setBounds(halfWidth, 85, halfWidth - 10, halfHeight + 40);
}

void MainComponent::drawNextLineOfSpectrogram()
{
    auto rightHandEdge = spectrogramImage.getWidth() - 1;
    auto imageHeight = spectrogramImage.getHeight();
    spectrogramImage.moveImageSection(0, 0, 1, 0, rightHandEdge, imageHeight);

    auto fftData = fft.getFFTData();
    int fftSize = fft.getFFTSize();
        
    auto fftRange = juce::FloatVectorOperations::findMinAndMax(fftData.data(), fftSize / 2);

    for (auto y = 1; y < imageHeight; ++y)
    {
        float skewedProportionY = 1.0f - std::exp(std::log((float)y / (float)imageHeight) * 0.2f);
        size_t fftDataIndex = (size_t)juce::jlimit(0, fftSize / 2, (int)(skewedProportionY * fftSize / 2 ));
        float level = juce::jmap(fftData[fftDataIndex], 0.0f, juce::jmax(fftRange.getEnd(), 1e-5f), 0.0f, 1.0f);

        spectrogramImage.setPixelAt(rightHandEdge, y, juce::Colour::fromHSV(level, 1.0f, level, 1.0f));
    }
}

void MainComponent::calcNote()
{
    // Getting fundamental frequency from FFT and calculating midi note number with velocity.
    auto fund = fft.calcFundamentalFreq();
    double amp = fund.second;
    unsigned int note = findNearestNote(fund.first);
    int velocity = (int)(amp * 127.0);

    std::vector<bool> noteValues;
    if (determineNoteValue(note, amp, noteValues))
    {
        for (bool value : noteValues)
        {
            if (value == true)
            {
                createMidiMsg(note, velocity, value);
            }
            else
            {
                // Problems with turning correct midi note off led to just turning
                // everything off. This works for monophonic playing.
                midiOut->sendMessageNow(juce::MidiMessage::allNotesOff(midiChannel));
            }
        }
    }
}

bool MainComponent::determineNoteValue(const unsigned int& note, const double& amp, std::vector<bool>& noteValues)
{
    // Ensures that notes are within midi range.
    if (note >= 0 && note < 128)
    {
        if (!midiNoteCurrentlyOn && amp > threshold)
        {
            // When there's no note currently playing, and the note
            // surpasses the threshold.
            noteValues.push_back(true); // Note on
            midiNoteCurrentlyOn = true;
            lastNote = note;
            lastAmp = amp;
            return true;
        }
        if (midiNoteCurrentlyOn)
        {
            // When another note is currently playing.
            if (note != lastNote)
            {
                // When new note is differetn from the last note.
                if (amp > threshold)
                {
                    // When new, different note surpasses threshold.
                    // Last note is turned off before new note is turned on.
                    noteValues.push_back(false); // Note off
                    noteValues.push_back(true); // Note on
                    midiNoteCurrentlyOn = true;
                    lastNote = note;
                    lastAmp = amp;
                    return true;
                }
                return false;
            }
            if (amp > lastAmp * 2)
            {
                // When new note is the same as last note,
                // it has to be sufficiently louder to retrigger.
                if (amp > threshold)
                {
                    noteValues.push_back(false); // Note off
                    noteValues.push_back(true); // Note on
                    midiNoteCurrentlyOn = true;
                    lastNote = note;
                    lastAmp = amp;
                    return true;
                }
                return false;
            }
            if (amp < releaseThreshold)
            {
                // When new note is not different, nor louder than last
                // it's probably a releasing note and we check if it has rung out.
                noteValues.push_back(false); // Note off
                midiNoteCurrentlyOn = false;
                return true;
            }
        }
    }
    return false;
}


void MainComponent::createMidiMsg(const unsigned int& noteNum, const juce::uint8& velocity, const bool noteOn)
{
    juce::MidiMessage midiMessage;
    if (noteOn)
    {
        midiMessage = juce::MidiMessage::noteOn(midiChannel, noteNum, velocity);
    }
    else
    {
        midiMessage = juce::MidiMessage::noteOff(midiChannel, noteNum);
    }

    midiMessage.setTimeStamp(juce::Time::getMillisecondCounter() * 0.001 - startTime);
    addMessageToBuffer(midiMessage);
    DBG(midiMessage.getDescription());
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

int MainComponent::findNearestNote(double target)
{
    unsigned int begin = 0;
    unsigned int end = noteFrequencies.size();
    
    // When frequency is below or above highest midi note.
    if (target <= noteFrequencies[begin])
    {
        return begin;
    }
    if (target >= noteFrequencies[end - 1])
    {
        return end - 1;
    }

    // Variation of binary search.
    unsigned int mid;
    while (end - begin > 1)
    {
        mid = begin + (end - begin) / 2;

        if (target == noteFrequencies[mid])
        {
            return mid;
        }
        if (target < noteFrequencies[mid])
        {
            end = mid;
        }
        else
        {
            begin = mid;
        }
    }

    if (target - noteFrequencies[begin] < target - noteFrequencies[end])
    {
        return begin;
    }
    else
    {
        return end;
    }
}

void MainComponent::addMessageToBuffer(const juce::MidiMessage& message)
{
    double timestamp = message.getTimeStamp();
    int sampleRate = deviceManager.getAudioDeviceSetup().sampleRate;
    int sampleNumber = (int)(timestamp * sampleRate);

    midiBuffer.addEvent(message, sampleNumber);
}