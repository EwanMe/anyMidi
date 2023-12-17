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

anyMidi::MainComponent::MainComponent(const juce::ValueTree &v)
    : tree_{v}, gui_{v} {
    addAndMakeVisible(gui_);
    setSize(width, height);
}

void anyMidi::MainComponent::paint(juce::Graphics &g) {
    // (Our component is opaque, so we must completely fill the background with
    // a solid colour)
    g.fillAll(
        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void anyMidi::MainComponent::resized() { gui_.setBounds(getLocalBounds()); }