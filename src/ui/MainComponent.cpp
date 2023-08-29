/**
 *
 *  @file      MainComponent.cpp
 *  @author    Hallvard Jensen
 *  @date      13 Feb 2021
 *  @copyright Hallvard Jensen, 2021. All right reserved.
 *
 */

#include "../ui/MainComponent.h"

#include "../core/AudioProcessor.h"
#include "../util/Globals.h"
#include "UserInterface.h"

using namespace anyMidi;

MainComponent::MainComponent(juce::ValueTree v) : tree{v}, gui{v} {
    addAndMakeVisible(gui);
    setSize(400, 290);
}

void MainComponent::paint(juce::Graphics &g) {
    // (Our component is opaque, so we must completely fill the background with
    // a solid colour)
    g.fillAll(
        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized() { gui.setBounds(getLocalBounds()); }