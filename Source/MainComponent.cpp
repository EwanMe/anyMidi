#include "MainComponent.h"
#include <fstream>

//==============================================================================
MainComponent::MainComponent() :
    fft{48000},
    spectrogramImage(juce::Image::RGB, 512, 512, true),
    audioSetupComp (
        deviceManager,
        0,      // min input ch
        256,    // max input ch
        0,      // min output ch
        256,    // max output ch
        true,  // can select midi inputs?
        true,  // can select midi output device?
        false,  // treat channels as stereo pairs
        false)  // hide advanced options?
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
    gainSlider.setValue(0.8);

    // Output box, used for debugging.
    addAndMakeVisible(midiOutputBox);
    midiOutputBox.setMultiLine(true);
    midiOutputBox.setReturnKeyStartsNewLine(true);
    midiOutputBox.setReadOnly(true);
    midiOutputBox.setScrollbarsShown(true);
    midiOutputBox.setCaretVisible(false);
    midiOutputBox.setPopupMenuEnabled(true);
    midiOutputBox.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x32ffffff));
    midiOutputBox.setColour(juce::TextEditor::outlineColourId, juce::Colour(0x1c000000));
    midiOutputBox.setColour(juce::TextEditor::shadowColourId, juce::Colour(0x16000000));

    addAndMakeVisible(clearOutput);
    clearOutput.setButtonText("Clear output");
    clearOutput.onClick = [this]
    {
        midiOutputBox.clear();
    };

    // Timer used for FFT spectrum analysis.
    startTimerHz(60);

    setSize(1000, 600);
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

    
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
    if (bufferToFill.buffer->getNumChannels() > 0)
    {
        auto* channelData = bufferToFill.buffer->getReadPointer(0, bufferToFill.startSample);
        auto* outBuffer_1 = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
        auto* outBuffer_2 = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);

        for (unsigned int i = 0; i < bufferToFill.numSamples; ++i)
        {
            fft.pushNextSampleIntoFifo(channelData[i]);
            
            // Throughput for debugging purposes. Just sends singnal through, scaled bu the gain slider.
            outBuffer_1[i] = channelData[i] * gainSlider.getValue();
            outBuffer_2[i] = channelData[i] * gainSlider.getValue();
        }
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
    if (fft.nextFFTBlockReady)
    {
        // Draws the spectrogram image.
        drawNextLineOfSpectrogram();
        calcNote();
        fft.nextFFTBlockReady = false;
        repaint();
    }
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
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.

    auto rect = getLocalBounds();
    
    auto halfWidth = getWidth() / 2;
    auto halfHeight = getHeight() / 2;

    audioSetupComp.setBounds(rect.withWidth(halfWidth));

    clearOutput.setBounds(rect.getCentreX(), 160, halfWidth / 2 - 10, 20);

    gainSlider.setBounds(rect.getCentreX(), 100, halfWidth / 2 - 10, 20);

    midiOutputBox.setBounds(halfWidth, halfHeight, halfWidth - 10, halfHeight - 10);
}

void MainComponent::drawNextLineOfSpectrogram()
{
    auto rightHandEdge = spectrogramImage.getWidth() - 1;
    auto imageHeight = spectrogramImage.getHeight();
    spectrogramImage.moveImageSection(0, 0, 1, 0, rightHandEdge, imageHeight);

    auto fftData = fft.getFFTData();
    int fftSize = fft.getFFTSize();
        
    auto fftRange = juce::FloatVectorOperations::findMinAndMax(fftData->data(), fftSize / 2);

    for (auto y = 1; y < imageHeight; ++y)
    {
        float skewedProportionY = 1.0f - std::exp(std::log((float)y / (float)imageHeight) * 0.2f);
        size_t fftDataIndex = (size_t)juce::jlimit(0, fftSize / 2, (int)(skewedProportionY * fftSize / 2 ));
        float level = juce::jmap(fftData->at(fftDataIndex), 0.0f, juce::jmax(fftRange.getEnd(), 1e-5f), 0.0f, 1.0f);

        spectrogramImage.setPixelAt(rightHandEdge, y, juce::Colour::fromHSV(level, 1.0f, level, 1.0f));
    }
}

void MainComponent::calcNote()
{
    // Getting fundamental frequency from FFT and calculating midi note number.
    double fundFreq = std::floor(fft.calcFundamentalFreq());
    unsigned int fundNote = static_cast<unsigned int>(std::floor(log2(fundFreq / 440.0) / log2(2) * 12 + 69));
    
    // Ensures that notes are within midi range.
    if (fundNote <= 128)
    {
        createMidiMsg(fundNote, 127);
    }
}


void MainComponent::createMidiMsg(const unsigned int& noteNum, const juce::uint8& velocity)
{
    auto midiMessage{ juce::MidiMessage::noteOn(midiChannels, noteNum, velocity) };
    log(midiMessage);
}

void MainComponent::log(const juce::MidiMessage& midiMessage)
{
    midiOutputBox.moveCaretToEnd();
    midiOutputBox.insertTextAtCaret(midiMessage.getDescription() + juce::newLine);
    DBG(midiMessage.getDescription());
}

void MainComponent::log(juce::String msg)
{
    midiOutputBox.moveCaretToEnd();
    midiOutputBox.insertTextAtCaret(msg);
}