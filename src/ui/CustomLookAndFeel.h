/**
 *
 *  @file      CustomLookAndFeel.h
 *  @brief     GUI theme definitions.
 *  @author    Hallvard Jensen
 *  @date      8 Aug 2021
 *  @copyright Hallvard Jensen, 2021. All right reserved.
 *
 */

#pragma once

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace anyMidi {

/**
 *
 *  @class   CustomLookaAndFeel
 *  @brief   Custom JUCE theme for application.
 *
 */
class CustomLookAndFeel : public juce::LookAndFeel_V4 {
public:
    CustomLookAndFeel();

    enum ColorGroup {
        BackgroundPrimary,
        BackgroundSecondary,
        TextPrimary,
        TextSecondary,
        Outline,
        Active
    };

private:
    const juce::Colour black_{juce::Colour(10, 10, 10)};
    const juce::Colour gray_{juce::Colour(33, 33, 36)};
    const juce::Colour lightGray_{juce::Colour(100, 100, 100)};
    const juce::Colour white_{juce::Colour(241, 241, 241)};
    const juce::Colour blue_{juce::Colour(5, 151, 250)};

    const std::map<ColorGroup, juce::Colour> palette_ =
        std::map<ColorGroup, juce::Colour>{
            {ColorGroup::BackgroundPrimary, gray_},
            {ColorGroup::BackgroundSecondary, black_},
            {ColorGroup::TextPrimary, white_},
            {ColorGroup::TextSecondary, lightGray_},
            {ColorGroup::Outline, lightGray_},
            {ColorGroup::Active, blue_}};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomLookAndFeel)
};
}; // namespace anyMidi