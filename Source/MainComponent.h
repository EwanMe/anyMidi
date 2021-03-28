#pragma once

#include <JuceHeader.h>
#include <fstream>
#include "ForwardFFT.h"


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
    //======= AUDIO ===========

    juce::AudioDeviceSelectorComponent audioSetupComp;

    static constexpr unsigned int numInputChannels{ 1 };
    static constexpr unsigned int numOutputChannels{ 2 };
    
    // Forward FFT object.
    ForwardFFT fft;

    // Lookup array to determine Midi notes from frequencies.
    std::array<double, 128> noteFrequencies;
    unsigned int tuning{ 440 };

    // Determines values for Midi message based on FFT analysis.
    void calcNote();

    // Finds closest Midi note in array of frequencies.
    int findNearestNote(double target);
    
    //======= AUDIO-END =======
    
    //======= MIDI ============

    juce::MidiBuffer midiBuffer;
    juce::MidiOutput* midiOut;
    const int midiChannels{ 10 };
    int previoudSampleNum{ 0 };
    int currentMidiBufferSize{ 0 };

    // Audio app start time. Used to determine Midi message timestamp.
    const double startTime;

    // Creates MIDI message based on input note number and velocity.
    void createMidiMsg(const unsigned int& noteNum, const juce::uint8& velocity);

    // Adds Midi message into buffer to be retrieved upon callback.
    void addMessageToBuffer(const juce::MidiMessage& message);
    
    //======= MIDI-END ========


    // Logging function for debugging purposes.
    void log(const juce::MidiMessage& midiMessage);
    
    template<typename T>
    void log(T msg);


    // ====== LAYOUT ==========

    juce::TextButton clearOutput;
    juce::Slider gainSlider;
    juce::TextEditor outputBox;
    juce::Image spectrogramImage;

    // ====== LAYOUT-END ======


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
