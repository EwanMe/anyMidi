#pragma once

#include <JuceHeader.h>
#include "ForwardFFT.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent, private juce::Timer
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    void timerCallback() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;
    void drawNextLineOfSpectrogram();

private:
    //==============================================================================
    //====== Channels =====

    static constexpr unsigned int numInputChannels{ 1 };
    static constexpr unsigned int numOutputChannels{ 2 };
    const int midiChannels = 10;

    // ====== Layout ======

    juce::TextButton clearOutput;

    juce::Slider gainSlider;
    juce::TextEditor midiOutputBox;

    juce::AudioDeviceSelectorComponent audioSetupComp;

    // Determines values for Midi message based on FFT analysis.
    void calcNote();

    // Creates MIDI message based on inputed note number and velocity,
    // and sends it to the output list.
    void createMidiMsg(const unsigned int& noteNum, const juce::uint8& velocity);

    // Logging function for debugging purposes.
    void log(const juce::MidiMessage& midiMessage);
    void log(juce::String msg);

    juce::Image spectrogramImage;

    ForwardFFT fft;

    bool toPrint{ false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
