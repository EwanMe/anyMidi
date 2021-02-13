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
        setAudioChannels (2, 0);
    }

    addAndMakeVisible(createMidiButton);
    createMidiButton.setButtonText("Create MIDI note");
    createMidiButton.onClick = [this] {
        constexpr unsigned int offset { 12 }; // For some reason, JUCE's Midi Messages are off by one octave.
        int noteVal = noteInput.getTextValue().getValue();
        setNoteNum(noteVal + offset, velocitySlider.getValue());
    };

    addAndMakeVisible(velocitySlider);
    velocitySlider.setRange(0, 127, 1);
    velocitySlider.onValueChange = [this]
    {
        auto midiMessage = juce::MidiMessage::controllerEvent(midiChannels, 7, (int)velocitySlider.getValue());
        addToOutputList(midiMessage);
    };

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
    bufferToFill.clearActiveBufferRegion();
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
    auto midiMessage{ juce::MidiMessage::noteOn(midiChannels, noteNum, velocity) };
    addToOutputList(midiMessage);
}

void MainComponent::addToOutputList(const juce::MidiMessage& midiMessage) {
    midiOutputBox.moveCaretToEnd();
    midiOutputBox.insertTextAtCaret(midiMessage.getDescription() + juce::newLine);
}