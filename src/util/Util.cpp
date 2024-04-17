#include <chrono>

#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>

#include "../core/AudioProcessor.h"
#include "Constants.h"
#include "Util.h"

namespace {
/**
 * @brief  Get the application config directory.
 *
 * @throws runtime_error if config subdirectory could not be created.
 */
juce::File getConfigDir() {
#ifdef NDEBUG
    static const auto AnyMidiConfigDir =
        juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("anyMidi");
#else
    static const auto AnyMidiConfigDir =
        juce::File::getSpecialLocation(juce::File::currentExecutableFile)
            .getSiblingFile("config");
#endif
    if (!AnyMidiConfigDir.exists()) {
        auto result = AnyMidiConfigDir.createDirectory();
        if (result.failed()) {
            throw std::runtime_error(result.getErrorMessage().toStdString());
        }
    }
    return AnyMidiConfigDir;
}

/**
 * @brief Remove a named child from a value tree.
 *
 * @param parentTree Value tree to remove from
 * @param name       Name of child to remove
 */
void removeChildWithName(juce::ValueTree &parentTree,
                         const juce::Identifier &name) {
    for (int i = 0; i < parentTree.getNumChildren(); ++i) {
        auto child = parentTree.getChild(i);
        if (child.hasType(name)) {
            parentTree.removeChild(i, nullptr);
            return;
        }
    }
}

/**
 * @brief Write value tree to file.
 *
 * @param tree  Value tree to store
 * @param file  File to write to
 * @param debug Whether to log the operations
 */
void writeSettingsToFile(const juce::ValueTree &tree, const juce::File &file,
                         bool debug = false) {
    auto xml = tree.toXmlString();

    // Save state as xml file to local dir.
    if (file.replaceWithText(xml) && debug) {
        anyMidi::log(tree, std::format("State written to {}",
                                       file.getFullPathName().toStdString()));
    } else if (debug) {
        anyMidi::log(tree, "Failed to write state to file.");
    }
}
} // anonymous namespace

juce::File anyMidi::getAudioSettingsFile() {
    return getConfigDir().getChildFile(AUDIO_SETTINGS_FILENAME);
}

juce::File anyMidi::getTimestampedAppSettingsFile() {
    // Get current time (chrono is pretty!)
    const std::chrono::time_point timePoint(std::chrono::system_clock::now());
    auto dayPoint = std::chrono::floor<std::chrono::days>(timePoint);
    std::chrono::year_month_day ymd(dayPoint);
    const std::chrono::hh_mm_ss hms(
        std::chrono::floor<std::chrono::milliseconds>(timePoint - dayPoint));

    std::stringstream timestamp;
    timestamp << std::format("{:%Y-%m-%d}", ymd) << "_"
              << std::format("{:%H-%M-%OS}", hms);
    auto filename = "anyMidi_state_" + timestamp.str() + ".xml";

    return getConfigDir().getChildFile(filename);
}

juce::File anyMidi::getAppSettingsFile() {
    return getConfigDir().getChildFile("anyMidi_config.xml");
}

void anyMidi::storeDebugSettings(juce::ValueTree &tree) {
    // Reference Counted Objects can't be serialized into XML.
    // Device manager is stored and the node it lies in is replaced with a
    // string while XML is generated. After, the device manager is added
    // back into the tree as a RCO.
    auto root = tree.getRoot();
    auto audioProcNode = root.getChildWithName(anyMidi::AUDIO_PROC_ID);
    auto *deviceManager = dynamic_cast<anyMidi::AudioDeviceManagerRCO *>(
        audioProcNode.getProperty(anyMidi::DEVICE_MANAGER_ID).getObject());

    if (deviceManager) {
        audioProcNode.setProperty(
            anyMidi::DEVICE_MANAGER_ID,
            "Audio device manager exists, settings in separate file.", nullptr);
    } else {
        audioProcNode.setProperty(anyMidi::DEVICE_MANAGER_ID,
                                  "No audio device manager found.", nullptr);
    }

    writeSettingsToFile(root, anyMidi::getTimestampedAppSettingsFile());

    // Reset the altered node.
    audioProcNode.setProperty(anyMidi::DEVICE_MANAGER_ID, deviceManager,
                              nullptr);
}

void anyMidi::storeSettings(juce::ValueTree &tree) {
    auto guiNode = tree.getRoot().getChildWithName(anyMidi::GUI_ID);
    guiNode.removeProperty(anyMidi::LOG_ID, nullptr);
    removeChildWithName(guiNode, anyMidi::ALL_WIN_ID);

    writeSettingsToFile(guiNode, anyMidi::getAppSettingsFile());
}
