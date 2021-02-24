#include "MainComponent.h"
#include <fstream>

//==============================================================================
MainComponent::MainComponent() : 
    forwardFFT(fftOrder),
    // When initialising the windowing function, consider using fftSize + 1, ref. https://artandlogic.com/2019/11/making-spectrograms-in-juce/amp/
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
    // Some platforms require permissions to open input channels so request that here
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
            const auto storedSettings = juce::parseXML(deviceSettingsFile);
            setAudioChannels(numInputChannels, numOutputChannels, storedSettings.get());
        }
        else
        {
            setAudioChannels(numInputChannels, numOutputChannels);
        }
    }
    
    addAndMakeVisible(audioSetupComp);

    addAndMakeVisible(createMidiButton);
    createMidiButton.setButtonText("Create MIDI note");
    createMidiButton.onClick = [this] {
        constexpr unsigned int offset { 12 }; // For some reason, JUCE's Midi Messages are off by one octave.
        int noteVal = noteInput.getTextValue().getValue();
        setNoteNum((noteVal + offset), velocitySlider.getValue());
    };

    addAndMakeVisible(velocitySlider);
    velocitySlider.setRange(0, 127, 1);

    addAndMakeVisible(gainSlider);
    gainSlider.setRange(0, 1, 0.01);
    gainSlider.setValue(0.8);

    addAndMakeVisible(noteInput);
    noteInput.setMultiLine(false);
    noteInput.setCaretVisible(true);
    noteInput.setReadOnly(false);

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

    // Make sure you set the size of the component after
    // you add any child components.
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
            pushNextSampleIntoFifo(channelData[i]);
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
    if (nextFFTBlockReady)
    {
        drawNextLineOfSpectrogram();
        nextFFTBlockReady = false;
        repaint();
    }
}

void MainComponent::pushNextSampleIntoFifo(float sample)
{
    // When fifo contains enough data, flag is set to say next frame should be rendered.
    if (fifoIndex == fftSize)
    {
        if (!nextFFTBlockReady)
        {
            std::fill(fftData.begin(), fftData.end(), 0.0f);
            std::copy(fifo.begin(), fifo.end(), fftData.begin());
            nextFFTBlockReady = true;
        }

        fifoIndex = 0;
    }

    fifo[fifoIndex++] = sample;
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.drawImage(spectrogramImage, getLocalBounds().toFloat());
    // You can add your drawing code here!

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
    
    createMidiButton.setBounds(rect.getCentreX(), 10, halfWidth/2 - 10, 20);
    noteInput.setBounds(rect.getCentreX(), 40, halfWidth/2 - 10, 20);
    velocitySlider.setBounds(rect.getCentreX(), 70, halfWidth/2 - 10, 20);
    gainSlider.setBounds(rect.getCentreX(), 100, halfWidth / 2 - 10, 20);

    midiOutputBox.setBounds(halfWidth, halfHeight, halfWidth - 10, halfHeight - 10);
}

void MainComponent::drawNextLineOfSpectrogram()
{
    auto rightHandEdge = spectrogramImage.getWidth() - 1;
    auto imageHeight = spectrogramImage.getHeight();

    spectrogramImage.moveImageSection(0, 0, 1, 0, rightHandEdge, imageHeight);

    forwardFFT.performRealOnlyForwardTransform(fftData.data());

    addToOutputList(std::to_string(*fftData.data()) + '\n');

    auto maxLevel = juce::FloatVectorOperations::findMinAndMax(fftData.data(), fftSize / 2);

    for (auto y = 1; y < imageHeight; ++y)
    {
        auto skewedProportionY = 1.0f - std::exp(std::log((float)y / (float)imageHeight) * 0.2f);
        auto fftDataIndex = (size_t)juce::jlimit(0, fftSize / 2, (int)(skewedProportionY * fftSize / 2));
        auto level = juce::jmap(fftData[fftDataIndex], 0.0f, juce::jmax(maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);

        spectrogramImage.setPixelAt(rightHandEdge, y, juce::Colour::fromHSV(level, 1.0f, level, 1.0f));
    }
}

void MainComponent::setNoteNum(const unsigned int& noteNum, const juce::uint8& velocity)
{
    // Creates MIDI message based on inputed note number and velocity,
    // and sends it to the output list.

    auto midiMessage{ juce::MidiMessage::noteOn(midiChannels, noteNum, velocity) };
    addToOutputList(midiMessage);
}

void MainComponent::addToOutputList(const juce::MidiMessage& midiMessage)
{
    // Displays midi messages to the output list.
    
    midiOutputBox.moveCaretToEnd();
    midiOutputBox.insertTextAtCaret(midiMessage.getDescription() + juce::newLine);
    DBG(midiMessage.getDescription());
}

void MainComponent::addToOutputList(juce::String msg)
{
    // Displays general messages (debugging) to the output list.

    midiOutputBox.moveCaretToEnd();
    midiOutputBox.insertTextAtCaret(msg);
}