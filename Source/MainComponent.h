#pragma once

#include <JuceHeader.h>
#include <fstream>
#include "appGUI.h"

namespace anyMidi {
    //==================================================================================
    class MainComponent : public juce::Component
    {
    public:
        //==============================================================================
        MainComponent(juce::ValueTree v);

        //==============================================================================
        void paint(juce::Graphics& g) override;
        void resized() override;

    private:
        //==============================================================================
        juce::ValueTree tree;
        std::unique_ptr<anyMidi::TabbedComp> gui;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
    };
};// namespace anyMidi
