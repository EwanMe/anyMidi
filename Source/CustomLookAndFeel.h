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

        enum ColorGroup
        {
            BackgroundPrimary,
            BackgroundSecondary,
            TextPrimary,
            TextSecondary,
            Outline,
            Active
        };


    private:

        const juce::Colour black        { juce::Colour(10, 10, 10) };
        const juce::Colour gray         { juce::Colour(33, 33, 36) };
        const juce::Colour lightGray    { juce::Colour(100, 100, 100) };
        const juce::Colour white        { juce::Colour(241, 241, 241) };
        const juce::Colour blue         { juce::Colour(5, 151, 250) };

        const std::map<ColorGroup, juce::Colour> palette = std::map<ColorGroup, juce::Colour>
        {
            { ColorGroup::BackgroundPrimary, gray },
            { ColorGroup::BackgroundSecondary, black },
            { ColorGroup::TextPrimary, white },
            { ColorGroup::TextSecondary, lightGray },
            { ColorGroup::Outline, lightGray },
            { ColorGroup::Active, blue }
        };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomLookaAndFeel)

    };
}; // namespace anyMidi