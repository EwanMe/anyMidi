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
};