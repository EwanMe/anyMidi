#pragma once

#include <JuceHeader.h>
#include <fstream>
#include "ForwardFFT.h"
#include "MidiProcessor.h"


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

    anyMidi::ForwardFFT fft;
    anyMidi::MidiProcessor midiProc;
    
    juce::AudioDeviceSelectorComponent audioSetupComp;

    juce::AudioSampleBuffer processingBuffer;
    
    juce::dsp::NoiseGate<float> noiseGate;
    juce::IIRFilter hiPassFilter;

    //juce::dsp::NoiseGate< noiseGate;

    static constexpr unsigned int numInputChannels{ 1 };
    static constexpr unsigned int numOutputChannels{ 2 };

    // Lookup array to determine Midi notes from frequencies.
    std::vector<double> noteFrequencies;
    static constexpr unsigned int tuning{ 440 };

    // Determines values for Midi message based on FFT analysis.
    void calcNote();

    // Returns note value based on analysis of harmonics.
    std::pair<int, double> analyzeHarmonics();  

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
