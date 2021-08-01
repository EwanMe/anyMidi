/*
  ==============================================================================

    Globals.h
    Created: 1 Jul 2021 3:52:32pm
    Author:  Hallvard Jensen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace anyMidi
{
    constexpr char AUDIO_SETTINGS_FILENAME[]            { "audio_device_settings.xml" };

    static const juce::Identifier ROOT_ID               { "App" };
    
    static const juce::Identifier AUDIO_PROC_ID         { "AudioProcessor" };
    static const juce::Identifier DEVICE_MANAGER_ID     { "DeviceManager" };
    
    static const juce::Identifier GUI_ID                { "GUI" };
    static const juce::Identifier ATTACK_THRESH_ID      { "AttackThreshold" };
    static const juce::Identifier RELEASE_THRESH_ID     { "ReleaseThreshold" };
    static const juce::Identifier PARTIALS_ID           { "NumParitals" };
    static const juce::Identifier LO_CUT_ID             { "LowCutFrequenzy" };
    static const juce::Identifier HI_CUT_ID             { "HighCutFrequenzy" };
    static const juce::Identifier LOG_ID                { "Log" };


    
    // Logs any non-object juce::var to output on the debug tab.
    inline void log(juce::ValueTree tree, juce::var message)
    {
        if (message.isObject())
        {
            message = "Error: Cannot log objects.";
        }
        tree.getRoot().getChildWithName(anyMidi::GUI_ID).setProperty(anyMidi::LOG_ID, message, nullptr);
    }
    
    // Logs any type with overloaded operator<< to debug tab.
    template<typename T>
    inline void log(juce::ValueTree tree, const T& message)
    {
        std::stringstream ss;
        ss << message;
        juce::var msgVar = ss.str();
        anyMidi::log(tree, msgVar);
    }
};