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
    static constexpr unsigned int numInputChannels{ 2 };
    static constexpr unsigned int numOutputChannels{ 2 };

    const int midiChannels = 10;
    juce::TextButton createMidiButton;
    juce::Slider velocitySlider;
    juce::Slider gainSlider;
    juce::TextEditor noteInput;
    juce::TextEditor midiOutputBox;

    juce::AudioDeviceSelectorComponent audioSetupComp;

    void setNoteNum(const unsigned int& noteNum, const juce::uint8& velocity);
    void addToOutputList(const juce::MidiMessage& midiMessage);
    void addToOutputList(juce::String msg);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
