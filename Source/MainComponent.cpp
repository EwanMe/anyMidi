#include "MainComponent.h"
#include "AudioProcessor.h"

using namespace anyMidi;

//==============================================================================
MainComponent::MainComponent(juce::ValueTree v) :
    gui{ v },
    tree{ v }
{
    juce::Identifier guiNodeType{ "GUI" };
    juce::ValueTree guiNode{ guiNodeType };
    //tree.setProperty(guiNode, -1, nullptr);

    

    addAndMakeVisible(gui);

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
}

void MainComponent::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    auto rect = getLocalBounds();

    auto halfWidth = getWidth() / 2;
    auto halfHeight = getHeight() / 2;

    clearOutput.setBounds(rect.getCentreX(), 15, halfWidth / 2 - 10, 20);

    outputBox.setBounds(halfWidth, 85, halfWidth - 10, halfHeight + 40);
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