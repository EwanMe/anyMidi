#pragma once

#include <juce_core/juce_core.h>

#include "Constants.h"

namespace anyMidi {

/**
 * @brief Get the audio settings file.
 */
juce::File getAudioSettingsFile();

/**
 * @brief Get app settings file whose name contains the current time.
 */
juce::File getTimestampedAppSettingsFile();

/**
 * @brief Get app settings file.
 */
juce::File getAppSettingsFile();

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

/**
 * @brief Writes root of value tree to timestamped file for debug. Removes all
 *        binary nodes, but restores the value tree to previous state after
 *        write.
 *
 * @param tree Value tree to store
 */
void storeDebugSettings(juce::ValueTree &tree);

/**
 * @brief Find GUI node and stores the value tree to file. Log and list of
 *        windowing functions needs not be persisted between restarts.
 *
 * @param tree The value tree to store
 */
void storeSettings(juce::ValueTree &tree);
} // namespace anyMidi