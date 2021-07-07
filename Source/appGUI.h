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
    // =============================================================================
    class TabbedComp : public juce::TabbedComponent
    {
    public:
        TabbedComp(juce::ValueTree v);

    private:
        juce::ValueTree tree;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TabbedComp)
    };

    // =============================================================================
    class AudioSetupPage : public juce::Component
    {
    public:
        AudioSetupPage(juce::ValueTree v);
        ~AudioSetupPage();
        void resized() override;

    private:
        std::unique_ptr<juce::AudioDeviceSelectorComponent> audioSetupComp;
        juce::ValueTree tree;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioSetupPage)
    };

    // =============================================================================
    class AppSettingsPage : public juce::Component
    {
    public:
        AppSettingsPage(juce::ValueTree v);
        void resized() override;

        juce::Slider attThreshSlider;
        juce::Label attThreshLabel;
        juce::Slider relThreshSlider;
        juce::Label relThreshLabel;
        juce::Slider partialsSlider;
        juce::Label partialsLabel;

        juce::TextEditor output;

    private:
        juce::ValueTree tree;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AppSettingsPage)
    };

}; // namespace anyMidi