/*
  ==============================================================================

    appGUI.cpp
    Created: 15 Jun 2021 12:27:45pm
    Author:  Hallvard Jensen

  ==============================================================================
*/

#include "appGUI.h"
#include "Globals.h"
#include "AudioProcessor.h"

using namespace anyMidi;

// =============================================================================
// TABBED COMPONENT

TabbedComp::TabbedComp(juce::ValueTree v) :
    TabbedComponent(juce::TabbedButtonBar::TabsAtTop),
    tree{ v }
{
    auto color = juce::Colour(0, 0, 0);
    addTab("App Settings", color, new AppSettingsPage(tree), true);
    addTab("Audio Settings", color, new AudioSetupPage(tree), true);
}

void TabbedComp::resized()
{
    this->setBounds(getLocalBounds().reduced(4));
}

// =============================================================================
// AUDIO SETUP PAGE

AudioSetupPage::AudioSetupPage(juce::ValueTree v) :
    tree{ v }        
{
    auto val = tree.getChildWithName(anyMidi::AUDIO_PROC_ID).getProperty(anyMidi::DEVICE_MANAGER_ID);
    auto deviceManager = dynamic_cast<anyMidi::AudioDeviceManagerRCO*>(val.getObject());
    audioSetupComp = std::make_unique<juce::AudioDeviceSelectorComponent>
        (
            *deviceManager,
            0,      // min input ch
            256,    // max input ch
            0,      // min output ch
            256,    // max output ch
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
    /*addAndMakeVisible(output);
    output.setReturnKeyStartsNewLine(true);
    output.setReadOnly(true);
    output.setScrollbarsShown(true);
    output.setCaretVisible(false);*/

    // ATTACK THRESHOLD SLIDER
    addAndMakeVisible(attThreshSlider);
    attThreshSlider.setRange(0, 1, 0.01);
    attThreshSlider.setSliderStyle(juce::Slider::LinearBarVertical);
    attThreshSlider.setColour(juce::Slider::ColourIds::trackColourId, juce::Colours::transparentWhite);
    attThreshSlider.setVelocityBasedMode(true);
    attThreshSlider.setVelocityModeParameters(0.4, 1, 0.09, false);

    // ATTACK THRESHOLD LABEL
    addAndMakeVisible(attThreshLabel);
    attThreshLabel.setText("Attack Threshold", juce::dontSendNotification);
    attThreshLabel.attachToComponent(&attThreshSlider, true);

    // RELEASE THRESHOLD SLIDER
    addAndMakeVisible(relThreshSlider);
    relThreshSlider.setRange(0, 1, 0.01);
    relThreshSlider.setSliderStyle(juce::Slider::LinearBarVertical);
    relThreshSlider.setColour(juce::Slider::ColourIds::trackColourId, juce::Colours::transparentWhite);
    relThreshSlider.setVelocityBasedMode(true);
    relThreshSlider.setVelocityModeParameters(0.4, 1, 0.09, false);

    // RELEASE THRESHOLD LABEL
    addAndMakeVisible(relThreshLabel);
    relThreshLabel.setText("Release Threshold", juce::dontSendNotification);
    relThreshLabel.attachToComponent(&relThreshSlider, true);

    // PARTIALS SLIDER
    addAndMakeVisible(partialsSlider);
    partialsSlider.setRange(0, 10, 1);
    partialsSlider.setSliderStyle(juce::Slider::LinearBarVertical);
    partialsSlider.setColour(juce::Slider::ColourIds::trackColourId, juce::Colours::transparentWhite);
    partialsSlider.setVelocityBasedMode(true);
    partialsSlider.setVelocityModeParameters(0.4, 1, 0.09, false);
    partialsSlider.onValueChange = [this]
    {
        tree.setProperty(anyMidi::PARTIALS_ID, partialsSlider.getValue(), nullptr);
    };

    // PARTIALS LABEL
    addAndMakeVisible(partialsLabel);
    partialsLabel.setText("Partials", juce::dontSendNotification);
    partialsLabel.attachToComponent(&partialsSlider, true);
}

void AppSettingsPage::resized()
{
    constexpr int buttonWidth = 100;
    constexpr int buttonHeight = 20;

    attThreshSlider.setBounds(100, 10, buttonWidth, buttonHeight);
    relThreshSlider.setBounds(100, 50, buttonWidth, buttonHeight);
    partialsSlider.setBounds(100, 90, buttonWidth, buttonHeight);


}