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
    class AudioProcessor; // Forward declaration

    // =============================================================================
    class TabbedComp : public juce::TabbedComponent
    {
    public:
        TabbedComp();
        void resized() override;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TabbedComp)
    };

    // =============================================================================
    class AudioSetupPage : public juce::Component
    {
    public:
        AudioSetupPage();
        ~AudioSetupPage();

        void resized() override;

    private:
        juce::AudioDeviceManager deviceManager;
        juce::AudioDeviceSelectorComponent audioSetupComp;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioSetupPage)
    };

    // =============================================================================
    class AppSettingsPage : public juce::Component
    {
    public:
        AppSettingsPage();

        void resized() override;

        juce::Slider attThreshSlider;
        juce::Label attThreshLabel;
        juce::Slider relThreshSlider;
        juce::Label relThreshLabel;
        juce::Slider partialsSlider;
        juce::Label partialsLabel;

        juce::TextEditor output;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AppSettingsPage)
    };

}; // namespace anyMidi