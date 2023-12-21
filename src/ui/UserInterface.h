/**
 *
 *  @file      UserInterface.h
 *  @brief     Various components making up the GUI of the application.
 *  @author    Hallvard Jensen
 *  @date      15 Jun 2021
 *  @copyright Hallvard Jensen, 2021. All right reserved.
 *
 */

#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include "../util/Globals.h"
#include "CustomLookAndFeel.h"

namespace anyMidi {

/**
 * User interface constants.
 */

constexpr int elementWidth = 100;
constexpr int elementHeight = 20;
constexpr int buttonWidth = elementWidth;
constexpr int buttonHeight = 30;
constexpr int yPad = 30;
constexpr int xPad = 20;
constexpr int labelPad = xPad;

/**
 *
 *  @class   AudioSetupPage
 *  @brief   GUI wrapper around the JUCE AudioDeviceSelectorComponent,
 *           connecting it with the rest of the GUI and the ValueTree.
 *
 */
class AudioSetupPage : public juce::Component {
public:
    explicit AudioSetupPage(const juce::ValueTree &v);
    ~AudioSetupPage() override;

    void resized() override;

private:
    std::unique_ptr<juce::AudioDeviceSelectorComponent> audioSetupComp_;
    juce::ValueTree tree_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioSetupPage)
};

/**
 *
 *  @class   AppSettingsPage
 *  @brief   Settings page for interaction with the FFT analysis algorithm,
 *           enabling adjustments of general parameters.
 *
 */
class AppSettingsPage : public juce::Component {
public:
    explicit AppSettingsPage(const juce::ValueTree &v);
    ~AppSettingsPage() override = default;

    void resized() override;

private:
    static constexpr double frequenzyLowerBound{20.0};
    static constexpr double frequenzyUpperBound{20000.0};

    juce::Slider attThreshSlider_;
    juce::Slider relThreshSlider_;
    juce::Slider partialsSlider_;
    juce::Slider filterSlider_;
    juce::TextEditor loCutFreq_;
    juce::TextEditor hiCutFreq_;
    juce::ComboBox winMethodList_;

    juce::Label attThreshLabel_;
    juce::Label relThreshLabel_;
    juce::Label partialsLabel_;
    juce::Label filterLabel_;
    juce::Label winMethodLabel_;

    juce::ValueTree tree_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AppSettingsPage)
};

/**
 *
 *  @class   DebugPage
 *  @brief   GUI for debugging purposes, providing logging functionality
 *           and access to the ValueTree serialization.
 *
 */
class DebugPage : public juce::Component, public juce::ValueTree::Listener {
public:
    explicit DebugPage(const juce::ValueTree &v);
    ~DebugPage() override = default;

    void resized() override;

    void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                  const juce::Identifier &property) override;

private:
    juce::ValueTree tree_;
    juce::TextEditor outputBox_;
    juce::TextButton clearOutput_;
    juce::TextButton writeToXml_;

    juce::Label outputBoxLabel_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DebugPage)
};

/**
 *
 *  @class   TabbedComp
 *  @brief   Main UI component of the application, providing different tabs
 *           for the different application interactions the user can do.
 *
 */
class TabbedComp : public juce::TabbedComponent {
public:
    explicit TabbedComp(const juce::ValueTree &v);
    ~TabbedComp() override = default;

private:
    juce::ValueTree tree_;

    juce::Viewport audioSetupViewport_;
    AudioSetupPage audioSetupPage_;
    AppSettingsPage appSettingsPage_;
    DebugPage debugPage_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TabbedComp)
};

/**
 *
 *  @class   TrayIcon
 *  @brief   Icon that sits in the system tray, giving access to the controls of
 *           the application.
 *
 */
class TrayIcon : public juce::SystemTrayIconComponent {
public:
    juce::DocumentWindow *mainWindow;

    explicit TrayIcon(juce::DocumentWindow *mainWindow);

    /**
     *  @brief Opens a popup menu for application interaction when tray icon is
     *         clicked.
     *  @param  - Mouse event, unused.
     */
    void mouseDown(const juce::MouseEvent &mouseEvent) override;
};

}; // namespace anyMidi