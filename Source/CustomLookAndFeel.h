/*
  ==============================================================================

    CustomLookAndFeel.h
    Created: 8 Aug 2021 3:09:40pm
    Author:  Hallvard Jensen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace anyMidi {
    class CustomLookaAndFeel : public juce::LookAndFeel_V4
    {
    public:
        CustomLookaAndFeel();

    private:
        const juce::Colour black        { juce::Colour(34, 40, 49) };
        const juce::Colour gray         { juce::Colour(57, 62, 70) };
        const juce::Colour white        { juce::Colour(238, 238, 238) };
        const juce::Colour dimWhite     { white.darker(0.5) };
        const juce::Colour turquoise    { juce::Colour(127, 216, 190) };
        const juce::Colour gold         { juce::Colour(255, 211, 105) };
    };
}; // namespace anyMidi