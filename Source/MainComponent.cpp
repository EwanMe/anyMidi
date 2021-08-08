#include "MainComponent.h"
#include "AudioProcessor.h"
#include "Globals.h"
#include "appGUI.h"

using namespace anyMidi;

//==============================================================================
MainComponent::MainComponent(juce::ValueTree v) :
    tree{ v },
    gui{ v }
{ 
    addAndMakeVisible(gui);
    setLookAndFeel(&layout);
    setSize(500, 400);
}

MainComponent::~MainComponent()
{
    setLookAndFeel(nullptr);
}

void MainComponent::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    gui.setBounds(getLocalBounds());
}