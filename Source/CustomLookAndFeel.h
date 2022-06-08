/**
 *
 *  @file      CustomLookAndFeel.h
 *  @brief     GUI theme definitions.
 *  @author    Hallvard Jensen
 *  @date      8 Aug 2021 3:09:40pm
 *  @copyright © Hallvard Jensen, 2021. All right reserved.
 *
 */

#pragma once

#include <JuceHeader.h>

namespace anyMidi {

    /**
     *
     *  @class   CustomLookaAndFeel
     *  @brief   Custom JUCE theme for application.
     *
     */
    class CustomLookaAndFeel : public juce::LookAndFeel_V4
    {

    public:

        CustomLookaAndFeel();


    private:

        const juce::Colour black        { juce::Colour(10, 10, 10) };
        const juce::Colour gray         { juce::Colour(41, 41, 41) };
        const juce::Colour lightGray    { juce::Colour(71, 71, 71) };
        const juce::Colour white        { juce::Colour(251, 255, 254) };
        const juce::Colour blue         { juce::Colour(68, 157, 209) };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomLookaAndFeel)

    };
}; // namespace anyMidi