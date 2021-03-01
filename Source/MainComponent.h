#pragma once

#include <JuceHeader.h>

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
    void pushNextSampleIntoFifo(float sample);

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;
    void drawNextLineOfSpectrogram();

    enum
    {
        fftOrder = 11,
        fftSize = 1 << fftOrder,
        scopeSize = 512
    };

private:
    //==============================================================================
    //====== Channels =====
    static constexpr unsigned int numInputChannels{ 1 };
    static constexpr unsigned int numOutputChannels{ 2 };
    const int midiChannels = 10;

    // ====== Layout ======
    juce::TextButton createMidiButton;
    juce::Slider velocitySlider;
    juce::Slider gainSlider;
    juce::TextEditor noteInput;
    juce::TextEditor midiOutputBox;

    juce::AudioDeviceSelectorComponent audioSetupComp;

    void setNoteNum(const unsigned int& noteNum, const juce::uint8& velocity);
    void addToOutputList(const juce::MidiMessage& midiMessage);
    void addToOutputList(juce::String msg);

    // ====== FFT ======
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;
    juce::Image spectrogramImage;

    float fifo[fftSize];
    float fftData[2 * fftSize];
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
