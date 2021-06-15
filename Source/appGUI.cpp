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

anyMidiTabbedComp::anyMidiTabbedComp(juce::AudioDeviceManager& deviceManager) : TabbedComponent(juce::TabbedButtonBar::TabsAtTop)
{
    auto color = juce::Colour(0, 0, 0);

    addTab("Audio Settings", color, new audioSetupPage(deviceManager), true);
    addTab("App Settings", color, new appSettingsPage(), true);
}

// =============================================================================
// AUDIO SETUP PAGE

audioSetupPage::audioSetupPage(juce::AudioDeviceManager& deviceManager) :
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

audioSetupPage::~audioSetupPage()
{
    auto audioDeviceSettings = audioSetupComp.deviceManager.createStateXml();

    if (audioDeviceSettings != nullptr)
    {
        // Writes user settings to XML file for storage.
        juce::File settingsFileName = juce::File::getCurrentWorkingDirectory().getChildFile("audio_device_settings.xml");
        settingsFileName.replaceWithText(audioDeviceSettings->toString());
    }
}

void audioSetupPage::resized()
{
    audioSetupComp.setBounds(getLocalBounds().withWidth(getWidth()));
}

// =============================================================================
// APP SETTINGS PAGE

appSettingsPage::appSettingsPage()
{

}

void appSettingsPage::resized()
{

}