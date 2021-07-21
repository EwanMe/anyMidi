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