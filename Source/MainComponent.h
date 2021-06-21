#pragma once

#include <JuceHeader.h>
#include <fstream>
#include "ForwardFFT.h"
#include "MidiProcessor.h"

constexpr char AUDIO_SETTINGS_FILENAME[] = "audio_device_settings.xml";

namespace anyMidi
{
    class TabbedComp;
};

class MainComponent  : public juce::AudioAppComponent, public juce::Slider::Listener
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

    void setAttackThreshold(double thresh);
    void setReleaseThreshold(double thresh);
    void setNumPartials(unsigned int num);

    void sliderValueChanged(juce::Slider* slider) override;
    void sliderDragStarted(juce::Slider*) override;
    void sliderDragEnded(juce::Slider*) override;

    //==============================================================================
    // Logging function for debugging purposes.
    void log(const juce::MidiMessage& midiMessage);

    template<typename T>
    void log(T msg);

private:
    //==============================================================================
    anyMidi::ForwardFFT fft;
    anyMidi::MidiProcessor midiProc;

    juce::AudioSampleBuffer processingBuffer;
    
    juce::dsp::NoiseGate<float> noiseGate;
    juce::IIRFilter hiPassFilter;

    //juce::dsp::NoiseGate< noiseGate;

    static constexpr unsigned int numInputChannels{ 1 };
    static constexpr unsigned int numOutputChannels{ 2 };

    //==============================================================================
    // Lookup array to determine Midi notes from frequencies.
    std::vector<double> noteFrequencies;
    static constexpr unsigned int tuning{ 440 }; // Can't be changed due to the MIDI protocol.

    // Determines values for Midi message based on FFT analysis.
    void calcNote();

    // Returns note value based on analysis of harmonics.
    std::pair<int, double> analyzeHarmonics();
    unsigned int numPartials{ 6 };

    //==============================================================================
    juce::TextButton clearOutput;
    juce::Slider gainSlider;
    juce::TextEditor outputBox;

    std::unique_ptr<anyMidi::TabbedComp> gui;



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
