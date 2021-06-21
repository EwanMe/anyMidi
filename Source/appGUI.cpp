/*
  ==============================================================================

    appGUI.cpp
    Created: 15 Jun 2021 12:27:45pm
    Author:  Hallvard Jensen

  ==============================================================================
*/

#include "appGUI.h"

using namespace anyMidi;

// =============================================================================
// TABBED COMPONENT

TabbedComp::TabbedComp(MainComponent* mc) : TabbedComponent(juce::TabbedButtonBar::TabsAtTop)
{
    addAndMakeVisible(this);

    auto color = juce::Colour(0, 0, 0);

    addTab("Audio Settings", color, new AudioSetupPage(mc->deviceManager), true);
    addTab("App Settings", color, new AppSettingsPage(mc), true);
}

void TabbedComp::resized()
{
    this->setBounds(getLocalBounds().reduced(4));
}

// =============================================================================
// AUDIO SETUP PAGE

AudioSetupPage::AudioSetupPage(juce::AudioDeviceManager& deviceManager) :
    audioSetupComp{
        deviceManager,
        0,      // min input ch
        256,    // max input ch
        0,      // min output ch
        256,    // max output ch
        false,  // can select midi inputs?
        true,   // can select midi output device?
        false,  // treat channels as stereo pairs
        false } // hide advanced options?
{
    addAndMakeVisible(audioSetupComp);
}

AudioSetupPage::~AudioSetupPage()
{
    auto audioDeviceSettings = audioSetupComp.deviceManager.createStateXml();

    if (audioDeviceSettings != nullptr)
    {
        // Writes user settings to XML file for storage.
        juce::File settingsFileName = juce::File::getCurrentWorkingDirectory().getChildFile(AUDIO_SETTINGS_FILENAME);
        settingsFileName.replaceWithText(audioDeviceSettings->toString());
    }
}

void AudioSetupPage::resized()
{
    audioSetupComp.setBounds(getLocalBounds().withWidth(getWidth()));
}

// =============================================================================
// APP SETTINGS PAGE

AppSettingsPage::AppSettingsPage(MainComponent* mc)
{
    addAndMakeVisible(output);
    output.setReturnKeyStartsNewLine(true);
    output.setReadOnly(true);
    output.setScrollbarsShown(true);
    output.setCaretVisible(false);

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
    partialsSlider.onValueChange = [mc]
    {
        mc->DBG("Hello");
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