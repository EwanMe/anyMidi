/*
  ==============================================================================

    CustomLookAndFeel.cpp
    Created: 8 Aug 2021 3:09:40pm
    Author:  Hallvard Jensen

  ==============================================================================
*/

#include "CustomLookAndFeel.h"

using namespace anyMidi;

CustomLookaAndFeel::CustomLookaAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, juce::Colours::yellow); // remove

    setColour(juce::TabbedComponent::backgroundColourId, gray);
    setColour(juce::TabbedButtonBar::tabTextColourId, dimWhite);
    setColour(juce::TabbedButtonBar::frontTextColourId, white);

    setColour(juce::Slider::textBoxTextColourId, turquoise);
    setColour(juce::Slider::trackColourId, turquoise);
}