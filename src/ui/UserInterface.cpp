/**
 *
 *  @file      UserInterface.cpp
 *  @author    Hallvard Jensen
 *  @date      15 Jun 2021
 *  @copyright Hallvard Jensen, 2021. All right reserved.
 *
 */

#include <BinaryData.h>
#include <chrono>
#include <format>

#include "../core/AudioProcessor.h"
#include "../util/Globals.h"
#include "UserInterface.h"

anyMidi::TabbedComp::TabbedComp(const juce::ValueTree &v)
    : TabbedComponent(juce::TabbedButtonBar::TabsAtTop), tree_{v},
      audioSetupPage_{v}, appSettingsPage_{v}, debugPage_{v} {
    // audioSetupViewport.setViewedComponent(&audioSetupPage, false);
    // addAndMakeVisible(audioSetupPage);

    auto color =
        getLookAndFeel().findColour(juce::TabbedComponent::backgroundColourId);

    addTab("App Settings", color, &appSettingsPage_, true);
    addTab("Audio Settings", color, &audioSetupPage_, true);
    addTab("Debug", color, &debugPage_, true);

    // audioSetupViewport.setBounds(getLocalBounds());
    // audioSetupPage.setBounds(0, 0, 400, 290);
}

anyMidi::AudioSetupPage::AudioSetupPage(const juce::ValueTree &v) : tree_{v} {
    // Fetch audio device manager from the value tree.
    auto *deviceManager = dynamic_cast<anyMidi::AudioDeviceManagerRCO *>(
        tree_.getParent()
            .getChildWithName(anyMidi::AUDIO_PROC_ID)
            .getProperty(anyMidi::DEVICE_MANAGER_ID)
            .getObject());

    audioSetupComp_ = std::make_unique<juce::AudioDeviceSelectorComponent>(
        *deviceManager,
        0,     // min input ch
        256,   // max input ch // NOLINT
        0,     // min output ch
        0,     // max output ch
        false, // can select midi inputs?
        true,  // can select midi output device?
        false, // treat channels as stereo pairs
        false  // hide advanced options?
    );

    addAndMakeVisible(*audioSetupComp_);
}

anyMidi::AudioSetupPage::~AudioSetupPage() {
    auto audioDeviceSettings = audioSetupComp_->deviceManager.createStateXml();

    if (audioDeviceSettings != nullptr) {
        // Writes user settings to XML file for storage.
        const juce::File settingsFileName =
            juce::File::getCurrentWorkingDirectory().getChildFile(
                anyMidi::AUDIO_SETTINGS_FILENAME);
        settingsFileName.replaceWithText(audioDeviceSettings->toString());
    }
}

void anyMidi::AudioSetupPage::resized() {
    audioSetupComp_->setBounds(getLocalBounds().withWidth(getWidth()));
}

anyMidi::AppSettingsPage::AppSettingsPage(const juce::ValueTree &v) : tree_{v} {
    constexpr double sliderSense{0.4};
    constexpr double sliderVelThresh{1};
    constexpr double sliderVelOffset{0.09};

    // Attack threshold slider
    constexpr double attThreshMin{0};
    constexpr double attThreshMax{1};
    constexpr double attThreshIncr{0.001};
    addAndMakeVisible(attThreshSlider_);
    attThreshSlider_.setRange(attThreshMin, attThreshMax, attThreshIncr);
    attThreshSlider_.setSliderStyle(juce::Slider::LinearBarVertical);
    attThreshSlider_.setColour(juce::Slider::ColourIds::trackColourId,
                               juce::Colours::transparentWhite);
    attThreshSlider_.setVelocityBasedMode(true);
    attThreshSlider_.setVelocityModeParameters(sliderSense, sliderVelThresh,
                                               sliderVelOffset, false);

    if (tree_.hasProperty(anyMidi::ATTACK_THRESH_ID)) {
        attThreshSlider_.setValue(tree_.getProperty(anyMidi::ATTACK_THRESH_ID));
    }

    // Callback
    attThreshSlider_.onValueChange = [this] {
        tree_.setProperty(anyMidi::ATTACK_THRESH_ID,
                          attThreshSlider_.getValue(), nullptr);
    };

    // Release threshold slider
    constexpr double relThreshMin{0};
    constexpr double relThreshMax{1};
    constexpr double relThreshIncr{0.001};
    addAndMakeVisible(relThreshSlider_);
    relThreshSlider_.setRange(relThreshMin, relThreshMax, relThreshIncr);
    relThreshSlider_.setSliderStyle(juce::Slider::LinearBarVertical);
    relThreshSlider_.setColour(juce::Slider::ColourIds::trackColourId,
                               juce::Colours::transparentWhite);
    relThreshSlider_.setVelocityBasedMode(true);
    relThreshSlider_.setVelocityModeParameters(sliderSense, sliderVelThresh,
                                               sliderVelOffset, false);

    if (tree_.hasProperty(anyMidi::RELEASE_THRESH_ID)) {
        relThreshSlider_.setValue(
            tree_.getProperty(anyMidi::RELEASE_THRESH_ID));
    }

    // Callback
    relThreshSlider_.onValueChange = [this] {
        tree_.setProperty(anyMidi::RELEASE_THRESH_ID,
                          relThreshSlider_.getValue(), nullptr);
    };

    // Partials slider
    constexpr double partialsMin{1};
    constexpr double partialsMax{10};
    constexpr double partialsIncr{1};
    addAndMakeVisible(partialsSlider_);
    partialsSlider_.setRange(partialsMin, partialsMax, partialsIncr);
    partialsSlider_.setSliderStyle(juce::Slider::LinearBarVertical);
    partialsSlider_.setColour(juce::Slider::ColourIds::trackColourId,
                              juce::Colours::transparentWhite);
    partialsSlider_.setVelocityBasedMode(true);
    partialsSlider_.setVelocityModeParameters(sliderSense, sliderVelThresh,
                                              sliderVelOffset, false);

    if (tree_.hasProperty(anyMidi::PARTIALS_ID)) {
        partialsSlider_.setValue(tree_.getProperty(anyMidi::PARTIALS_ID));
    }

    // Callback
    partialsSlider_.onValueChange = [this] {
        tree_.setProperty(anyMidi::PARTIALS_ID,
                          static_cast<int>(partialsSlider_.getValue()),
                          nullptr);
    };

    // Filter sliders
    constexpr int filterSliderWidth{300};
    constexpr int filterSliderHeight{1000};
    constexpr double frequenzyIncrement{1};
    addAndMakeVisible(filterSlider_);
    filterSlider_.setRange(frequenzyLowerBound, frequenzyUpperBound,
                           frequenzyIncrement);
    filterSlider_.setSliderStyle(juce::Slider::TwoValueHorizontal);
    filterSlider_.setTextBoxStyle(juce::Slider::NoTextBox, true,
                                  filterSliderWidth, filterSliderHeight);

    if (tree_.hasProperty(anyMidi::LO_CUT_ID) &&
        tree_.hasProperty(anyMidi::HI_CUT_ID)) {
        filterSlider_.setMinAndMaxValues(tree_.getProperty(anyMidi::LO_CUT_ID),
                                         tree_.getProperty(anyMidi::HI_CUT_ID));
    }

    // Callback
    filterSlider_.onValueChange = [this] {
        const juce::Value &loVal = filterSlider_.getMinValueObject();
        const juce::Value &hiVal = filterSlider_.getMaxValueObject();

        tree_.setProperty(anyMidi::LO_CUT_ID, loVal.getValue(), nullptr);
        tree_.setProperty(anyMidi::HI_CUT_ID, hiVal.getValue(), nullptr);

        loCutFreq_.setText(loVal.toString() + " Hz");
        hiCutFreq_.setText(hiVal.toString() + " Hz");
    };

    constexpr int maxTextLength{10};
    constexpr auto allowedCharacters = "0123456789.";

    // Low cut frequency
    addAndMakeVisible(loCutFreq_);
    loCutFreq_.setInputRestrictions(maxTextLength, allowedCharacters);
    loCutFreq_.setSelectAllWhenFocused(true);

    // Update min slider and format text on Enter
    loCutFreq_.onReturnKey = [this] {
        const double newValue =
            std::stod(loCutFreq_.getTextValue().toString().toStdString());

        std::stringstream ss;
        ss << std::fixed << std::setprecision(1);
        if (newValue <= filterSlider_.getMaxValue()) {
            filterSlider_.setMinValue(newValue);

            ss << newValue << " Hz";
            loCutFreq_.setText(ss.str());
        } else {
            ss << filterSlider_.getMinValue() << " Hz";
            loCutFreq_.setText(ss.str());
        }
    };

    // TODO: slider box not showing
    // High cut frequency
    addAndMakeVisible(hiCutFreq_);
    hiCutFreq_.setInputRestrictions(maxTextLength, allowedCharacters);
    hiCutFreq_.setSelectAllWhenFocused(true);

    // Update max slider and format text on Enter
    hiCutFreq_.onReturnKey = [this] {
        const double newValue =
            std::stod(hiCutFreq_.getTextValue().toString().toStdString());

        std::stringstream ss;
        ss << std::fixed << std::setprecision(1);
        if (newValue >= filterSlider_.getMinValue()) {
            filterSlider_.setMaxValue(newValue);

            ss << newValue << " Hz";
            hiCutFreq_.setText(ss.str());
        } else {
            ss << filterSlider_.getMaxValue() << " Hz";
            hiCutFreq_.setText(ss.str());
        }
    };

    // Windowing methods
    addAndMakeVisible(winMethodList_);
    auto winNode = tree_.getChildWithName(anyMidi::ALL_WIN_ID);
    for (int i = 0; i < winNode.getNumChildren(); ++i) {
        const auto &method =
            winNode.getChild(i).getProperty(anyMidi::WIN_NAME_ID);

        if (!method.toString().isEmpty()) {
            winMethodList_.addItem(method, i + 1);
        }
    }
    winMethodList_.setSelectedId(
        (int)tree_.getProperty(anyMidi::CURRENT_WIN_ID) + 1);

    winMethodList_.onChange = [this] {
        tree_.setProperty(anyMidi::CURRENT_WIN_ID,
                          winMethodList_.getSelectedId() - 1, nullptr);
    };

    // Attack threshold label
    addAndMakeVisible(attThreshLabel_);
    attThreshLabel_.setText("Attack thresh.", juce::dontSendNotification);

    // Release threshold label
    addAndMakeVisible(relThreshLabel_);
    relThreshLabel_.setText("Release thresh.", juce::dontSendNotification);

    // Partials label
    addAndMakeVisible(partialsLabel_);
    partialsLabel_.setText("Partials", juce::dontSendNotification);

    // Filter label
    addAndMakeVisible(filterLabel_);
    filterLabel_.setText("Filter", juce::dontSendNotification);

    // Windowing method label
    addAndMakeVisible(winMethodLabel_);
    winMethodLabel_.setText("Window", juce::dontSendNotification);
}

void anyMidi::AppSettingsPage::resized() {
    const int valPad = getWidth() / 3;

    constexpr int yOffsetLevel1{2};
    constexpr int yOffsetLevel2{4};
    constexpr int yOffsetLevel3{6};
    constexpr float yOffsetLevel4{7.2};
    constexpr int yOffsetLevel5{9};

    attThreshLabel_.setBounds(labelPad, yPad, elementWidth, elementHeight);
    relThreshLabel_.setBounds(labelPad, yPad + yOffsetLevel1 * elementHeight,
                              elementWidth, elementHeight);
    partialsLabel_.setBounds(labelPad, yPad + yOffsetLevel2 * elementHeight,
                             elementWidth, elementHeight);
    filterLabel_.setBounds(labelPad, yPad + yOffsetLevel3 * elementHeight,
                           elementWidth, elementHeight);
    winMethodLabel_.setBounds(labelPad, yPad + yOffsetLevel5 * elementHeight,
                              elementWidth * 2, elementHeight);

    attThreshSlider_.setBounds(valPad + elementWidth / 2, yPad, elementWidth,
                               elementHeight);
    relThreshSlider_.setBounds(valPad + elementWidth / 2,
                               yPad + yOffsetLevel1 * elementHeight,
                               elementWidth, elementHeight);
    partialsSlider_.setBounds(valPad + elementWidth / 2,
                              yPad + yOffsetLevel2 * elementHeight,
                              elementWidth, elementHeight);
    filterSlider_.setBounds(valPad, yPad + yOffsetLevel3 * elementHeight,
                            elementWidth * 2, elementHeight);
    loCutFreq_.setBounds(valPad,
                         yPad + static_cast<int>(yOffsetLevel4 * elementHeight),
                         elementWidth, elementHeight);
    hiCutFreq_.setBounds(valPad + elementWidth,
                         yPad + static_cast<int>(yOffsetLevel4 * elementHeight),
                         elementWidth, elementHeight);
    winMethodList_.setBounds(valPad, yPad + yOffsetLevel5 * elementHeight,
                             elementWidth * 2, elementHeight);
}

anyMidi::DebugPage::DebugPage(const juce::ValueTree &v) : tree_{v} {
    tree_.addListener(this);

    // Output box, used for debugging.
    addAndMakeVisible(outputBox_);
    outputBox_.setMultiLine(true);
    outputBox_.setReturnKeyStartsNewLine(true);
    outputBox_.setReadOnly(true);
    outputBox_.setScrollbarsShown(true);
    outputBox_.setCaretVisible(false);
    outputBox_.setPopupMenuEnabled(true);
    // outputBox.setComponentID("output");
    auto existingLog = tree_.getParent()
                           .getChildWithName(anyMidi::GUI_ID)
                           .getProperty(anyMidi::LOG_ID)
                           .toString();
    if (!existingLog.isEmpty()) {
        outputBox_.moveCaretToEnd();
        outputBox_.insertTextAtCaret(existingLog + juce::newLine);
    }

    addAndMakeVisible(outputBoxLabel_);
    outputBoxLabel_.setText("Output log:", juce::dontSendNotification);
    outputBoxLabel_.attachToComponent(&outputBox_, false);

    addAndMakeVisible(clearOutput_);
    clearOutput_.setButtonText("Clear output");
    clearOutput_.onClick = [this] { outputBox_.clear(); };

    addAndMakeVisible(writeToXml_);
    writeToXml_.setButtonText("Write state to file");
    writeToXml_.onClick = [this] {
        // Reference Counted Objects can't be serialized into XML.
        // Device manager is stored and the node it lies in is replaced with a
        // string while XML is generated. After, the device manager is added
        // back into the tree as a RCO.
        auto audioProcNode =
            tree_.getParent().getChildWithName(anyMidi::AUDIO_PROC_ID);
        auto *deviceManager = dynamic_cast<anyMidi::AudioDeviceManagerRCO *>(
            audioProcNode.getProperty(anyMidi::DEVICE_MANAGER_ID).getObject());

        if (deviceManager != nullptr) {
            audioProcNode.setProperty(
                anyMidi::DEVICE_MANAGER_ID,
                "Audio device manager exists, settings in separate file.",
                nullptr);
        } else {
            audioProcNode.setProperty(anyMidi::DEVICE_MANAGER_ID,
                                      "No audio device manager found.",
                                      nullptr);
        }

        auto xml = tree_.getRoot().toXmlString();

        // This holy mess to get a simple timestamp.
        const std::chrono::time_point timePoint(
            std::chrono::system_clock::now());
        auto dayPoint = std::chrono::floor<std::chrono::days>(timePoint);
        std::chrono::year_month_day ymd(dayPoint);
        const std::chrono::hh_mm_ss hms(
            std::chrono::floor<std::chrono::milliseconds>(timePoint -
                                                          dayPoint));

        std::stringstream timestamp;
        timestamp << std::format("{:%Y-%m-%d}", ymd) << "_"
                  << std::format("{:%H-%M-%OS}", hms);
        auto filename = "anyMidi_state_" + timestamp.str() + ".xml";

        // Save state as xml file to local dir.
        const auto file =
            juce::File::getCurrentWorkingDirectory().getChildFile(filename);
        if (file.replaceWithText(xml)) {
            anyMidi::log(tree_,
                         std::format("State successfully written to {}",
                                     file.getFullPathName().toStdString()));
        } else {
            anyMidi::log(tree_, "Failed to write state to file.");
        }

        // Reset the altered nodes.
        audioProcNode.setProperty(anyMidi::DEVICE_MANAGER_ID, deviceManager,
                                  nullptr);
    };
}

void anyMidi::DebugPage::resized() {
    const int outputWidth = getWidth() - 2 * xPad;
    const int outputHeight = (getHeight() / 3) * 2;
    constexpr float yPadScale{1.5};

    outputBox_.setBounds(xPad, yPad, outputWidth, outputHeight);
    clearOutput_.setBounds(xPad + (outputWidth / 2 - elementWidth) / 2,
                           (int)(yPadScale * yPad) + outputHeight, buttonWidth,
                           buttonHeight);
    writeToXml_.setBounds(
        xPad + outputWidth / 2 + (outputWidth / 2 - elementWidth) / 2,
        (int)(yPadScale * yPad) + outputHeight, buttonWidth, buttonHeight);
}

void anyMidi::DebugPage::valueTreePropertyChanged(
    juce::ValueTree &treeWhosePropertyHasChanged,
    const juce::Identifier &property) {
    if (property == anyMidi::LOG_ID) {
        // TODO: Color is not applied here, default color is used.
        outputBox_.applyColourToAllText(
            getLookAndFeel().findColour(juce::TextEditor::textColourId), true);

        const juce::String message =
            treeWhosePropertyHasChanged.getProperty(property).toString();
        outputBox_.moveCaretToEnd();
        outputBox_.insertTextAtCaret(message + juce::newLine);
    }
}

anyMidi::TrayIcon::TrayIcon(juce::DocumentWindow *mainWindow)
    : mainWindow{mainWindow} {
    const juce::Image icon = juce::ImageCache::getFromMemory(
        BinaryData::anyMidiLogo_png, BinaryData::anyMidiLogo_pngSize);
    const juce::Graphics g{icon};

    constexpr float trayIconWidth{8.0};
    constexpr float trayIconHeight{8.0};
    g.drawImage(icon, juce::Rectangle<float>(trayIconWidth, trayIconHeight));

    setIconImage(icon, icon);

    setIconTooltip("anyMidi");
}

void anyMidi::TrayIcon::mouseDown(
    [[maybe_unused]] const juce::MouseEvent &mouseEvent) {
    juce::PopupMenu menu;

    menu.setLookAndFeel(&(mainWindow->getLookAndFeel()));

    // Reopens the application window.
    menu.addItem("Open anyMidi", [=] { mainWindow->setVisible(true); });

    menu.addItem("Quit", [=] {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    });

    menu.showMenuAsync(juce::PopupMenu::Options());
}