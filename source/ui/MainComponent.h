/**
 *
 *  @file      MainComponent.h
 *  @brief     Main GUI component of the applocation.
 *  @author    Hallvard Jensen
 *  @date      13 Feb 2021 8:29:21 pm
 *  @copyright © Hallvard Jensen, 2021. All right reserved.
 *
 */

#pragma once

#include <JuceHeader.h>
#include "CustomLookAndFeel.h"
#include "UserInterface.h"

namespace anyMidi {

    // TODO:
    // MAKE MAINCOMPONENT A TABBED COMPONENT.

    /**
     *
     *  @class   MainComponent
     *  @brief   Main manager for the applications graphical user interface.
     *
     */
    class MainComponent : public juce::Component
    {

    public:
        MainComponent(juce::ValueTree v);
        
        ~MainComponent() override;

        void paint(juce::Graphics& g) override;
        
        void resized() override;


    private:

        juce::ValueTree tree;
        anyMidi::CustomLookaAndFeel layout;
        anyMidi::TabbedComp gui;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
    };
};// namespace anyMidi
