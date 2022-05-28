/*
  ==============================================================================

    appGUI.h
    Created: 15 Jun 2021 12:27:27pm
    Author:  Hallvard Jensen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace anyMidi {
    // =================================================================================
    class TabbedComp : public juce::TabbedComponent
    {
    public:
        //==============================================================================
        TabbedComp(juce::ValueTree v);

    private:
        //==============================================================================
        juce::ValueTree tree;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TabbedComp)
    };

    // =================================================================================
    class AudioSetupPage : public juce::Component
    {
    public:
        //==============================================================================
        AudioSetupPage(juce::ValueTree v);
        ~AudioSetupPage();
        void resized() override;

    private:
        //==============================================================================
        std::unique_ptr<juce::AudioDeviceSelectorComponent> audioSetupComp;
        juce::ValueTree tree;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioSetupPage)
    };

    // =================================================================================
    class AppSettingsPage : public juce::Component
    {
    public:
        //==============================================================================
        AppSettingsPage(juce::ValueTree v);
        void resized() override;


        juce::TextEditor output;

    private:
        //==============================================================================
        juce::Slider attThreshSlider;
        juce::Slider relThreshSlider;
        juce::Slider partialsSlider;
        juce::Slider filterSlider;
        juce::TextEditor loCutFreq;
        juce::TextEditor hiCutFreq;
        juce::ComboBox winMethodList;

        juce::Label attThreshLabel;
        juce::Label relThreshLabel;
        juce::Label partialsLabel;
        juce::Label filterLabel;
        juce::Label winMethodLabel;

        juce::ValueTree tree;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AppSettingsPage)
    };

    // =================================================================================
    class DebugPage :   public juce::Component,
                        public juce::ValueTree::Listener
    {
    public:
        //==============================================================================
        DebugPage(juce::ValueTree v);
        void resized() override;

        void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;

    private:
        //==============================================================================
        juce::ValueTree tree;
        juce::TextEditor outputBox;
        juce::TextButton clearOutput;
        juce::TextButton writeToXml;

        juce::Label outputBoxLabel;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DebugPage)
    };

}; // namespace anyMidi