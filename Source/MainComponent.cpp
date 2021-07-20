#include "MainComponent.h"
#include "AudioProcessor.h"
#include "Globals.h"
#include "appGUI.h"

using namespace anyMidi;

//==============================================================================
MainComponent::MainComponent(juce::ValueTree v) :
    tree{ v }
{ 
    gui = std::make_unique<anyMidi::TabbedComp>(tree);
    
    addAndMakeVisible(*gui);
    setSize(500, 500);
}

void MainComponent::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    gui->setBounds(getLocalBounds().reduced(4));
}

void MainComponent::log(const juce::MidiMessage& midiMessage)
{
    log(midiMessage.getDescription().toStdString());
}

template<typename T>
void MainComponent::log(const T& msg)
{
    auto outputBox = dynamic_cast<juce::TextEditor*>(gui->getTabContentComponent(2)->findChildWithID("output"));
    outputBox->moveCaretToEnd();
    outputBox->insertTextAtCaret(std::string(msg) + juce::newLine);
}