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
        const juce::Colour white        { juce::Colour(247, 245, 251) };
        const juce::Colour dimWhite     { juce::Colour::fromRGBA(247, 245, 251, 150) };
        const juce::Colour gray         { juce::Colour(47, 49, 55) };
        const juce::Colour turquoise    { juce::Colour(87, 226, 229) };
        const juce::Colour gold         { juce::Colour(200, 173, 85) };
    };
}; // namespace anyMidi