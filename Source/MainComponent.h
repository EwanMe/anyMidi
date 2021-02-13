#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    // Your private member variables go here...
    const int midiChannels = 10;
    juce::TextButton createMidiButton;
    juce::Slider velocitySlider;
    juce::TextEditor noteInput;
    juce::TextEditor midiOutputBox;

    void setNoteNum(const unsigned int& noteNum, const juce::uint8& velocity);
    void addToOutputList(const juce::MidiMessage& midiMessage);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
