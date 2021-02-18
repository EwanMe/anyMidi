#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
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
        // Specify the number of input and output channels that we want to open
        setAudioChannels (numInputChannels, numOutputChannels);
    }
    
    addAndMakeVisible(createMidiButton);
    createMidiButton.setButtonText("Create MIDI note");
    createMidiButton.onClick = [this] {
        constexpr unsigned int offset { 12 }; // For some reason, JUCE's Midi Messages are off by one octave.
        int noteVal = noteInput.getTextValue().getValue();
        setNoteNum((noteVal + offset), velocitySlider.getValue());
    };

    addAndMakeVisible(velocitySlider);
    velocitySlider.setRange(0, 1, 0.01);

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
    setSize(800, 600);
}

MainComponent::~MainComponent()
{
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
    auto* device = deviceManager.getCurrentAudioDevice();
    auto activeInputChannels = device->getActiveInputChannels();
    auto activeOutputChannels = device->getActiveOutputChannels();
    auto maxInputChannels = activeInputChannels.getHighestBit();
    auto maxOutputChannels = activeOutputChannels.getHighestBit();


    for (auto channel = 0; channel < maxOutputChannels; ++channel)
    {
        if ((!activeOutputChannels[channel]) || maxInputChannels == 0)
        {
            bufferToFill.buffer->clear(channel, bufferToFill.startSample, bufferToFill.numSamples);
        }
        else
        {
            auto actualInputChannel = channel % maxInputChannels;

            if (!activeInputChannels[channel])
            {
                bufferToFill.buffer->clear(channel, bufferToFill.startSample, bufferToFill.numSamples);
            }
            else
            {
                auto* inBuffer = bufferToFill.buffer->getReadPointer(actualInputChannel, bufferToFill.startSample);
                auto* outBuffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);

                for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
                    outBuffer[sample] = inBuffer[sample]*velocitySlider.getValue();
            }
        }
    }
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!

}

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    auto halfWidth = getWidth() / 2;

    auto buttonsBounds = getLocalBounds().withWidth(halfWidth).reduced(10);

    createMidiButton.setBounds(buttonsBounds.getX(), 10, buttonsBounds.getWidth(), 20);
    velocitySlider.setBounds(buttonsBounds.getX(), 220, buttonsBounds.getWidth(), 20);

    noteInput.setBounds(buttonsBounds.getX(), 40, buttonsBounds.getWidth(), 20);

    midiOutputBox.setBounds(getLocalBounds().withWidth(halfWidth).withX(halfWidth).reduced(10));
}

void MainComponent::setNoteNum(const unsigned int& noteNum, const juce::uint8& velocity) {
    // Creates MIDI message based on inputed note number and velocity,
    // and sends it to the output list.

    auto midiMessage{ juce::MidiMessage::noteOn(midiChannels, noteNum, velocity) };
    addToOutputList(midiMessage);
}

void MainComponent::addToOutputList(const juce::MidiMessage& midiMessage) {
    // Displays midi messages to the output list.
    std::cout << "hello" << std::endl;
    midiOutputBox.moveCaretToEnd();
    midiOutputBox.insertTextAtCaret(midiMessage.getDescription() + juce::newLine);
}

void MainComponent::addToOutputList(juce::String msg) {
    // Displays general messages (debugging) to the output list.

    midiOutputBox.moveCaretToEnd();
    midiOutputBox.insertTextAtCaret(msg);
}