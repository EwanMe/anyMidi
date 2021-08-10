/*
  ==============================================================================

    appGUI.cpp
    Created: 15 Jun 2021 12:27:45pm
    Author:  Hallvard Jensen

  ==============================================================================
*/

#include "UserInterface.h"
#include "Globals.h"
#include "AudioProcessor.h"

using namespace anyMidi;

// =============================================================================
// TABBED COMPONENT

TabbedComp::TabbedComp(juce::ValueTree v) :
    TabbedComponent(juce::TabbedButtonBar::TabsAtTop),
    tree{ v }
{
    auto color = findColour(juce::TabbedComponent::backgroundColourId);
    addTab("App Settings", color, new AppSettingsPage(tree), true);
    addTab("Audio Settings", color, new AudioSetupPage(tree), true);
    addTab("Debug", color, new DebugPage(tree), true);
}

// =============================================================================
// AUDIO SETUP PAGE

AudioSetupPage::AudioSetupPage(juce::ValueTree v) :
    tree{ v }        
{
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
            0,    // max output ch
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

// =============================================================================
// APP SETTINGS PAGE

AppSettingsPage::AppSettingsPage(juce::ValueTree v) :
    tree{ v }
{
    // ATTACK THRESHOLD SLIDER
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

    attThreshSlider.onValueChange = [this]
    {
        // Callback
        tree.setProperty(anyMidi::ATTACK_THRESH_ID, attThreshSlider.getValue(), nullptr);
    };

    // RELEASE THRESHOLD SLIDER
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

    relThreshSlider.onValueChange = [this]
    {
        // Callback
        tree.setProperty(anyMidi::RELEASE_THRESH_ID, relThreshSlider.getValue(), nullptr);
    };

    // PARTIALS SLIDER
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
    
    partialsSlider.onValueChange = [this]
    {
        // Callback
        tree.setProperty(anyMidi::PARTIALS_ID, static_cast<int>(partialsSlider.getValue()), nullptr);
    };

    // FILTER SLIDERS
    addAndMakeVisible(filterSlider);
    filterSlider.setRange(20, 20000, 1);
    filterSlider.setSliderStyle(juce::Slider::TwoValueHorizontal);
    // filterSlider.setVelocityBasedMode(true);
    filterSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 300, 1000);

    if (tree.hasProperty(anyMidi::LO_CUT_ID) && tree.hasProperty(anyMidi::HI_CUT_ID))
    {
        filterSlider.setMinAndMaxValues(tree.getProperty(anyMidi::LO_CUT_ID), tree.getProperty(anyMidi::HI_CUT_ID));
    }

    filterSlider.onValueChange = [this]
    {
        // Callback
        juce::Value& loVal = filterSlider.getMinValueObject();
        juce::Value& hival = filterSlider.getMaxValueObject();

        tree.setProperty(anyMidi::LO_CUT_ID, loVal.getValue(), nullptr);
        tree.setProperty(anyMidi::HI_CUT_ID, hival.getValue(), nullptr);

        loCutFreq.setText(loVal.toString() + " Hz");
        hiCutFreq.setText(hival.toString() + " Hz");
    };

    addAndMakeVisible(loCutFreq);
    loCutFreq.setReadOnly(true);

    addAndMakeVisible(hiCutFreq);
    hiCutFreq.setReadOnly(true);

    // ATTACK THRESHOLD LABEL
    addAndMakeVisible(attThreshLabel);
    attThreshLabel.setText("Attack Threshold", juce::dontSendNotification);
    attThreshLabel.attachToComponent(&attThreshSlider, true);

    // RELEASE THRESHOLD LABEL
    addAndMakeVisible(relThreshLabel);
    relThreshLabel.setText("Release Threshold", juce::dontSendNotification);
    relThreshLabel.attachToComponent(&relThreshSlider, true);

    // PARTIALS LABEL
    addAndMakeVisible(partialsLabel);
    partialsLabel.setText("Partials", juce::dontSendNotification);
    partialsLabel.attachToComponent(&partialsSlider, true);

    // FILTER LABEL
    addAndMakeVisible(filterLabel);
    filterLabel.setText("Filter", juce::dontSendNotification);
    filterLabel.attachToComponent(&filterSlider, true);
}

void AppSettingsPage::resized()
{
    constexpr int buttonWidth = 100;
    constexpr int buttonHeight = 20;

    attThreshSlider.setBounds(100, 10, buttonWidth, buttonHeight);
    relThreshSlider.setBounds(100, 50, buttonWidth, buttonHeight);
    partialsSlider.setBounds(100, 90, buttonWidth, buttonHeight);
    filterSlider.setBounds(100, 130, buttonWidth*2, buttonHeight);
    loCutFreq.setBounds(100, 150, buttonWidth, buttonHeight);
    hiCutFreq.setBounds(100+buttonWidth, 150, buttonWidth, buttonHeight);
}


// =============================================================================
// DEBUG PAGE

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
    outputBox.setComponentID("output");

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
    writeToXml.setButtonText("Write ValueTree to file");
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
        juce::File::getCurrentWorkingDirectory().getChildFile("ValueTree.xml").replaceWithText(xml);

        audioProcNode.setProperty(anyMidi::DEVICE_MANAGER_ID, deviceManager, nullptr);
    };
}

void DebugPage::resized()
{    
    outputBox.setBounds(10, 25, getWidth() - 20, getHeight() / 2);
    clearOutput.setBounds(10, getHeight() / 2 + 40, 100, 30);
    writeToXml.setBounds(10, getHeight() / 2 + 80, 100, 30);
}

void DebugPage::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
{
    if (property == anyMidi::LOG_ID)
    {
        juce::String message = treeWhosePropertyHasChanged.getProperty(property).toString();
        outputBox.moveCaretToEnd();
        outputBox.insertTextAtCaret(message + juce::newLine);
    }
}