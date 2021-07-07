#pragma once

#include <JuceHeader.h>
#include <fstream>
#include "appGUI.h"

//namespace anyMidi {

    class MainComponent : public juce::Component
    {
    public:
        //==============================================================================
        MainComponent();

        //==============================================================================
        void paint(juce::Graphics& g) override;
        void resized() override;

        //==============================================================================
        // Logging function for debugging purposes.
        void log(const juce::MidiMessage& midiMessage);

        template<typename T>
        void log(T msg);

    private:
        //==============================================================================
        juce::TextButton clearOutput;
        juce::Slider gainSlider;
        juce::TextEditor outputBox;

        anyMidi::TabbedComp gui;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
    };
//};// namespace anyMidi
