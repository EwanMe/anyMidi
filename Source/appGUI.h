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

    class anyMidiTabbedComp : public juce::TabbedComponent
    {
    public:
        anyMidiTabbedComp(juce::AudioDeviceManager& deviceManager);

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (anyMidiTabbedComp)
    };

    
    class audioSetupPage : public juce::Component
    {
    public:
        audioSetupPage(juce::AudioDeviceManager& deviceManager);
        ~audioSetupPage();
        void resized() override;

    private:
        juce::AudioDeviceSelectorComponent audioSetupComp;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (audioSetupPage)
    };


    class appSettingsPage : public juce::Component
    {
    public:
        appSettingsPage();
        void resized() override;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (appSettingsPage)
    };

}; // namespace anyMidi