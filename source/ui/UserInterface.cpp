/**
 *
 *  @file      UserInterface.cpp
 *  @author    Hallvard Jensen
 *  @date      15 Jun 2021 12:27:45pm
 *  @copyright © Hallvard Jensen, 2021. All right reserved.
 *
 */


#include "UserInterface.h"
#include "../util/Globals.h"
#include "../core/AudioProcessor.h"

using namespace anyMidi;


TabbedComp::TabbedComp(juce::ValueTree v) :
    TabbedComponent(juce::TabbedButtonBar::TabsAtTop),
    tree{ v },
    audioSetupPage{ v },
    appSettingsPage{ v },
    debugPage{ v }
{
    //audioSetupViewport.setViewedComponent(&audioSetupPage, false);
    //addAndMakeVisible(audioSetupPage);

    auto color = getLookAndFeel().findColour(juce::TabbedComponent::backgroundColourId);

    addTab("App Settings", color, &appSettingsPage, true);
    addTab("Audio Settings", color, &audioSetupPage, true);
    addTab("Debug", color, &debugPage, true);   

    //audioSetupViewport.setBounds(getLocalBounds());
    //audioSetupPage.setBounds(0, 0, 400, 290);
}


AudioSetupPage::AudioSetupPage(juce::ValueTree v) :
    tree{ v }        
{
    // Fetch audio device manager from the value tree.
    auto deviceManager = dynamic_cast<anyMidi::AudioDeviceManagerRCO*>
        (
            tree.getParent().getChildWithName(anyMidi::AUDIO_PROC_ID).getProperty(anyMidi::DEVICE_MANAGER_ID).getObject()
        );

    audioSetupComp = std::make_unique<juce::AudioDeviceSelectorComponent>
        (
            *deviceManager,
            0,      // min input ch
            256,    // max input ch
            0,      // min output ch
            0,      // max output ch
            false,  // can select midi inputs?
            true,   // can select midi output device?
            false,  // treat channels as stereo pairs
            false   // hide advanced options?
        );

    addAndMakeVisible(*audioSetupComp);
}


AudioSetupPage::~AudioSetupPage()
{
    auto audioDeviceSettings = audioSetupComp->deviceManager.createStateXml();

    if (audioDeviceSettings != nullptr)
    {
        // Writes user settings to XML file for storage.
        juce::File settingsFileName = juce::File::getCurrentWorkingDirectory().getChildFile(anyMidi::AUDIO_SETTINGS_FILENAME);
        settingsFileName.replaceWithText(audioDeviceSettings->toString());
    }
}


void AudioSetupPage::resized()
{
    audioSetupComp->setBounds(getLocalBounds().withWidth(getWidth()));
}


AppSettingsPage::AppSettingsPage(juce::ValueTree v) :
    tree{ v }
{
    // Attack treshold slider
    addAndMakeVisible(attThreshSlider);
    attThreshSlider.setRange(0, 1, 0.001);
    attThreshSlider.setSliderStyle(juce::Slider::LinearBarVertical);
    attThreshSlider.setColour(juce::Slider::ColourIds::trackColourId, juce::Colours::transparentWhite);
    attThreshSlider.setVelocityBasedMode(true);
    attThreshSlider.setVelocityModeParameters(0.4, 1, 0.09, false);
    
    if (tree.hasProperty(anyMidi::ATTACK_THRESH_ID))
    {
        attThreshSlider.setValue(tree.getProperty(anyMidi::ATTACK_THRESH_ID));
    }

    // Callback
    attThreshSlider.onValueChange = [this]
    {
        tree.setProperty(anyMidi::ATTACK_THRESH_ID, attThreshSlider.getValue(), nullptr);
    };

    // Release threshold slider
    addAndMakeVisible(relThreshSlider);
    relThreshSlider.setRange(0, 1, 0.001);
    relThreshSlider.setSliderStyle(juce::Slider::LinearBarVertical);
    relThreshSlider.setColour(juce::Slider::ColourIds::trackColourId, juce::Colours::transparentWhite);
    relThreshSlider.setVelocityBasedMode(true);
    relThreshSlider.setVelocityModeParameters(0.4, 1, 0.09, false);
    
    if (tree.hasProperty(anyMidi::RELEASE_THRESH_ID))
    {
        relThreshSlider.setValue(tree.getProperty(anyMidi::RELEASE_THRESH_ID));
    }

    // Callback
    relThreshSlider.onValueChange = [this]
    {
        tree.setProperty(anyMidi::RELEASE_THRESH_ID, relThreshSlider.getValue(), nullptr);
    };

    // Partials slider
    addAndMakeVisible(partialsSlider);
    partialsSlider.setRange(1, 10, 1);
    partialsSlider.setSliderStyle(juce::Slider::LinearBarVertical);
    partialsSlider.setColour(juce::Slider::ColourIds::trackColourId, juce::Colours::transparentWhite);
    partialsSlider.setVelocityBasedMode(true);
    partialsSlider.setVelocityModeParameters(0.4, 1, 0.09, false);
    
    if (tree.hasProperty(anyMidi::PARTIALS_ID))
    {
        partialsSlider.setValue(tree.getProperty(anyMidi::PARTIALS_ID));
    }
    
    // Callback
    partialsSlider.onValueChange = [this]
    {
        tree.setProperty(anyMidi::PARTIALS_ID, static_cast<int>(partialsSlider.getValue()), nullptr);
    };

    // Filter sliders
    addAndMakeVisible(filterSlider);
    filterSlider.setRange(20, 20000, 1);
    filterSlider.setSliderStyle(juce::Slider::TwoValueHorizontal);
    filterSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 300, 1000);

    if (tree.hasProperty(anyMidi::LO_CUT_ID) && tree.hasProperty(anyMidi::HI_CUT_ID))
    {
        filterSlider.setMinAndMaxValues(tree.getProperty(anyMidi::LO_CUT_ID), tree.getProperty(anyMidi::HI_CUT_ID));
    }

    // Callback
    filterSlider.onValueChange = [this]
    {
        juce::Value& loVal = filterSlider.getMinValueObject();
        juce::Value& hival = filterSlider.getMaxValueObject();

        tree.setProperty(anyMidi::LO_CUT_ID, loVal.getValue(), nullptr);
        tree.setProperty(anyMidi::HI_CUT_ID, hival.getValue(), nullptr);

        loCutFreq.setText(loVal.toString() + " Hz");
        hiCutFreq.setText(hival.toString() + " Hz");
    };

    // Low cut frequency
    addAndMakeVisible(loCutFreq);
    loCutFreq.setInputRestrictions(10, "0123456789.");
    loCutFreq.setSelectAllWhenFocused(true);
    
    // Update min slider and format text on Enter
    loCutFreq.onReturnKey = [this]
    {
        double newValue = std::stod(loCutFreq.getTextValue().toString().toStdString());

        std::stringstream ss;
        ss << std::fixed << std::setprecision(1);
        if (newValue <= filterSlider.getMaxValue())
        {
            filterSlider.setMinValue(newValue);

            ss << newValue << " Hz";
            loCutFreq.setText(ss.str());
        }
        else
        {
            ss << filterSlider.getMinValue() << " Hz";
            loCutFreq.setText(ss.str());
        }
    };
    
    // High cut frequency
    addAndMakeVisible(hiCutFreq);
    hiCutFreq.setInputRestrictions(10, "0123456789.");
    hiCutFreq.setSelectAllWhenFocused(true);

    // Update max slider and format text on Enter
    hiCutFreq.onReturnKey = [this]
    {
        double newValue = std::stod(hiCutFreq.getTextValue().toString().toStdString());

        std::stringstream ss;
        ss << std::fixed << std::setprecision(1);
        if (newValue >= filterSlider.getMinValue())
        {
            filterSlider.setMaxValue(newValue);

            ss << newValue << " Hz";
            hiCutFreq.setText(ss.str());
        }
        else
        {
            ss << filterSlider.getMaxValue() << " Hz";
            hiCutFreq.setText(ss.str());
        }
    };

    // Windowing methods
    addAndMakeVisible(winMethodList);
    auto winNode = tree.getChildWithName(anyMidi::ALL_WIN_ID);
    for (int i = 0; i < winNode.getNumChildren(); ++i)
    {
        auto& method = winNode.getChild(i).getProperty(anyMidi::WIN_NAME_ID);

        if (!method.toString().isEmpty())
        {
            winMethodList.addItem(method, i+1);
        }
    }
    winMethodList.setSelectedId((int) tree.getProperty(anyMidi::CURRENT_WIN_ID) + 1);

    winMethodList.onChange = [this]
    {
        tree.setProperty(anyMidi::CURRENT_WIN_ID, winMethodList.getSelectedId() - 1, nullptr);
    };

    // Attack threshold label
    addAndMakeVisible(attThreshLabel);
    attThreshLabel.setText("Attack thresh.", juce::dontSendNotification);

    // Release threshold label
    addAndMakeVisible(relThreshLabel);
    relThreshLabel.setText("Release thresh.", juce::dontSendNotification);

    // Partials label
    addAndMakeVisible(partialsLabel);
    partialsLabel.setText("Partials", juce::dontSendNotification);

    // Filter label
    addAndMakeVisible(filterLabel);
    filterLabel.setText("Filter", juce::dontSendNotification);

    // Windowing method label
    addAndMakeVisible(winMethodLabel);
    winMethodLabel.setText("Window", juce::dontSendNotification);
}


void AppSettingsPage::resized()
{
    const int valPad = getWidth() / 3;

    attThreshLabel.setBounds(labelPad, yPad, elementWidth, elementHeight);
    relThreshLabel.setBounds(labelPad, yPad + 2 * elementHeight, elementWidth, elementHeight);
    partialsLabel.setBounds(labelPad, yPad + 4 * elementHeight, elementWidth, elementHeight);
    filterLabel.setBounds(labelPad, yPad + 6 * elementHeight, elementWidth, elementHeight);
    winMethodLabel.setBounds(labelPad, yPad + 9 * elementHeight, elementWidth * 2, elementHeight);


    attThreshSlider.setBounds(valPad + elementWidth / 2, yPad, elementWidth, elementHeight);
    relThreshSlider.setBounds(valPad + elementWidth / 2, yPad + 2 * elementHeight, elementWidth, elementHeight);
    partialsSlider.setBounds(valPad + elementWidth / 2, yPad + 4 * elementHeight, elementWidth, elementHeight);
    filterSlider.setBounds(valPad, yPad + 6 * elementHeight, elementWidth * 2, elementHeight);
    loCutFreq.setBounds(valPad, static_cast<int>((double) yPad + 7.2) * elementHeight, elementWidth, elementHeight);
    hiCutFreq.setBounds(valPad + elementWidth, static_cast<int>((double) yPad + 7.2) * elementHeight, elementWidth, elementHeight);
    winMethodList.setBounds(valPad, yPad + 9 * elementHeight, elementWidth * 2, elementHeight);
}


DebugPage::DebugPage(juce::ValueTree v) :
    tree{ v }
{
    tree.addListener(this);

    // Output box, used for debugging.
    addAndMakeVisible(outputBox);
    outputBox.setMultiLine(true);
    outputBox.setReturnKeyStartsNewLine(true);
    outputBox.setReadOnly(true);
    outputBox.setScrollbarsShown(true);
    outputBox.setCaretVisible(false);
    outputBox.setPopupMenuEnabled(true); 
    //outputBox.setComponentID("output");
    auto existingLog = tree.getParent().getChildWithName(anyMidi::GUI_ID).getProperty(anyMidi::LOG_ID).toString();
    if (!existingLog.isEmpty())
    {
        outputBox.moveCaretToEnd();
        outputBox.insertTextAtCaret(existingLog + juce::newLine);
    }

    addAndMakeVisible(outputBoxLabel);
    outputBoxLabel.setText("Output log:", juce::dontSendNotification);
    outputBoxLabel.attachToComponent(&outputBox, false);

    addAndMakeVisible(clearOutput);
    clearOutput.setButtonText("Clear output");
    clearOutput.onClick = [this]
    {
        outputBox.clear();
    };

    addAndMakeVisible(writeToXml);
    writeToXml.setButtonText("Write state to file");
    writeToXml.onClick = [this]
    {
        // Reference Counted Objects can't be serialized into XML.
        // Device manager is stored and the node it lies in is replaced with a string while XML is generated.
        // After, the device manager is added back into the tree as a RCO.
        auto audioProcNode = tree.getParent().getChildWithName(anyMidi::AUDIO_PROC_ID);
        auto deviceManager = dynamic_cast<anyMidi::AudioDeviceManagerRCO*>
            (
                audioProcNode.getProperty(anyMidi::DEVICE_MANAGER_ID).getObject()
                );

        if (deviceManager != nullptr)
        {
            audioProcNode.setProperty(anyMidi::DEVICE_MANAGER_ID, "Audio device manager exists, settings in separate file.", nullptr);
        }
        else
        {
            audioProcNode.setProperty(anyMidi::DEVICE_MANAGER_ID, "No audio device manager found.", nullptr);
        }

        auto xml = tree.getRoot().toXmlString();
        
        // This holy mess to get a simple timestamp.
        std::chrono::time_point time_point(std::chrono::system_clock::now());
        auto day_point = std::chrono::floor<std::chrono::days>(time_point);
        std::chrono::year_month_day ymd(day_point);
        std::chrono::hh_mm_ss hms(std::chrono::floor<std::chrono::milliseconds>(time_point - day_point));

        std::stringstream timestamp;
        timestamp << std::format("{:%Y-%m-%d}", ymd) << "_" << std::format("{:%H-%M-%OS}", hms);
        
        // Save state as xml file to local dir.
        juce::File::getCurrentWorkingDirectory().getChildFile("anyMidi_state_" + timestamp.str() + ".xml").replaceWithText(xml);

        // Reset the altered nodes.
        audioProcNode.setProperty(anyMidi::DEVICE_MANAGER_ID, deviceManager, nullptr);
    };
}


void DebugPage::resized()
{    
    const int outputWidth = getWidth() - 2 * xPad;
    const int outputHeight = (getHeight() / 3) * 2;

    outputBox.setBounds(xPad, yPad, outputWidth, outputHeight);
    clearOutput.setBounds(xPad + (outputWidth / 2 - elementWidth) / 2, (int)(1.5 * yPad) + outputHeight, buttonWidth, buttonHeight);
    writeToXml.setBounds(xPad + outputWidth / 2 + (outputWidth / 2 - elementWidth) / 2, (int)(1.5 * yPad) + outputHeight, buttonWidth, buttonHeight);
}


void DebugPage::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
{
    if (property == anyMidi::LOG_ID)
    {
        // TODO: Color is not applied here, default color is used.
        outputBox.applyColourToAllText(getLookAndFeel().findColour(juce::TextEditor::textColourId), true);

        juce::String message = treeWhosePropertyHasChanged.getProperty(property).toString();
        outputBox.moveCaretToEnd();
        outputBox.insertTextAtCaret(message + juce::newLine);
    }
}


TrayIcon::TrayIcon(juce::DocumentWindow* mainWindow) : mainWindow{ mainWindow }
{
    juce::Image icon = juce::ImageCache::getFromMemory(BinaryData::anyMidiLogo_png, BinaryData::anyMidiLogo_pngSize);
    juce::Graphics g{ icon };
    g.drawImage(icon, juce::Rectangle<float>(8, 8));
    
    setIconImage(icon, icon);
    
    setIconTooltip("anyMidi");

}


void TrayIcon::mouseDown(const juce::MouseEvent&)
{
    juce::PopupMenu menu;

    menu.setLookAndFeel(&(mainWindow->getLookAndFeel()));

    // Reopens the application window.
    menu.addItem("Open anyMidi", [=]
        {
            mainWindow->setVisible(true);
        });

    menu.addItem("Quit", [=]
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        });

    menu.showMenuAsync(juce::PopupMenu::Options());
}