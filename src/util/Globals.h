/**
 *
 *  @file      Globals.h
 *  @brief     Collection of global constants and functions.
 *  @author    Hallvard Jensen
 *  @date      1 Jul 2021
 *  @copyright Hallvard Jensen, 2021. All right reserved.
 *
 */

#pragma once

#include <chrono>

#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>

namespace anyMidi {
static const char *AUDIO_SETTINGS_FILENAME = "audio_device_settings.xml";

static const juce::Identifier ROOT_ID{"App"};

static const juce::Identifier AUDIO_PROC_ID{"AudioProcessor"};
static const juce::Identifier DEVICE_MANAGER_ID{"DeviceManager"};

static const juce::Identifier GUI_ID{"GUI"};
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

#ifdef NDEBUG
static const juce::File CONFIG_DIR =
    juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
#else
static const juce::File CONFIG_DIR =
    juce::File::getSpecialLocation(juce::File::currentExecutableFile)
        .getParentDirectory();
#endif

static const juce::File AUDIO_SETTINGS_FILE =
    CONFIG_DIR.getChildFile(AUDIO_SETTINGS_FILENAME);

inline juce::File getTimestampedAppSettingsFile() {
    const std::chrono::time_point timePoint(std::chrono::system_clock::now());
    auto dayPoint = std::chrono::floor<std::chrono::days>(timePoint);
    std::chrono::year_month_day ymd(dayPoint);
    const std::chrono::hh_mm_ss hms(
        std::chrono::floor<std::chrono::milliseconds>(timePoint - dayPoint));

    std::stringstream timestamp;
    timestamp << std::format("{:%Y-%m-%d}", ymd) << "_"
              << std::format("{:%H-%M-%OS}", hms);
    auto filename = "anyMidi_state_" + timestamp.str() + ".xml";

    return anyMidi::CONFIG_DIR.getChildFile(filename);
}

inline juce::File getAppSettingsFile() {
    return anyMidi::CONFIG_DIR.getChildFile("anyMidi_config.xml");
}

/**
 *  @brief Logs any non-object juce::var to output on the debug tab.
 *  @param tree    - A value tree which contains a child with the log-property
 *                   to pass messages to.
 *  @param message - The message to log. Cannot be a juce object.
 */
inline void log(const juce::ValueTree &tree, juce::var message) {
    if (message.isObject()) {
        message = "Error: Cannot log objects.";
    }
    tree.getRoot()
        .getChildWithName(anyMidi::GUI_ID)
        .setProperty(anyMidi::LOG_ID, message, nullptr);
}

/**
 *  @brief  Logs any type with overloaded operator<< to debug tab.
 *  @tparam T       - Type of the message to be logged. The operator<< has to be
 *                    defined.
 *  @param  tree    - A value tree which contains a child with the log-property
 *                    to pass messages to.
 *  @param  message - The message to log.
 */
template <typename T>
inline void log(const juce::ValueTree &tree, const T &message) {
    std::stringstream ss;
    ss << message;
    juce::var msgVar = juce::String(ss.str());
    anyMidi::log(tree, msgVar);
}
}; // namespace anyMidi