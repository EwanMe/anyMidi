/**
 *
 *  @file      MainComponent.h
 *  @brief     Main GUI component of the applocation.
 *  @author    Hallvard Jensen
 *  @date      13 Feb 2021
 *  @copyright Hallvard Jensen, 2021. All right reserved.
 *
 */

#pragma once

#include <JuceHeader.h>

#include "CustomLookAndFeel.h"
#include "UserInterface.h"

namespace anyMidi {

/**
 *
 *  @class   MainComponent
 *  @brief   Main manager for the applications graphical user interface.
 *
 */
class MainComponent : public juce::Component {
public:
    explicit MainComponent(const juce::ValueTree &v);

    void paint(juce::Graphics &g) override;

    void resized() override;

private:
    juce::ValueTree tree_;
    anyMidi::TabbedComp gui_;

    static constexpr unsigned int width = 400;
    static constexpr unsigned int height = 290;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

}; // namespace anyMidi
