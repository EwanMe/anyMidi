#pragma once

#include <juce_data_structures/juce_data_structures.h>

namespace anyMidi {
static const juce::String AUDIO_SETTINGS_FILENAME = "audio_device_settings.xml";

static const juce::Identifier ROOT_ID{"AnyMidi"};

static const juce::Identifier AUDIO_PROC_ID{"AudioProcessor"};
static const juce::Identifier DEVICE_MANAGER_ID{"DeviceManager"};

static const juce::Identifier GUI_ID{"UserSettings"};
static const juce::Identifier ATTACK_THRESH_ID{"AttackThreshold"};
static const juce::Identifier RELEASE_THRESH_ID{"ReleaseThreshold"};
static const juce::Identifier PARTIALS_ID{"NumParitals"};
static const juce::Identifier LO_CUT_ID{"LowCutFrequenzy"};
static const juce::Identifier HI_CUT_ID{"HighCutFrequenzy"};
static const juce::Identifier LOG_ID{"Log"};

static const juce::Identifier ALL_WIN_ID{"AllWindowFunc"};
static const juce::Identifier CURRENT_WIN_ID{"CurrentWindowFunc"};
static const juce::Identifier WIN_NODE_ID{"Window"};
static const juce::Identifier WIN_NAME_ID{"WindowName"};

constexpr double msToSec{0.001};
constexpr double defaultSampleRate{48000};
} // namespace anyMidi