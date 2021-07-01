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

    static const juce::Identifier DEVICE_MANAGER_ID     { "deviceManager" };
    static const juce::Identifier PARTIALS_ID           { "numParitals" };
};