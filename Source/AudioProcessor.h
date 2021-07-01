/*
  ==============================================================================

    AudioProcessor.h
    Created: 22 Jun 2021 11:41:47pm
    Author:  Hallvard Jensen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ForwardFFT.h"
#include "MidiProcessor.h"

namespace anyMidi {

    class AudioProcessor : public juce::AudioSource
    {
    public:
        //==============================================================================
        AudioProcessor();
        AudioProcessor(AudioProcessor&& other) noexcept;
        AudioProcessor& operator=(AudioProcessor&& other) noexcept;
        ~AudioProcessor() override;

        void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
        void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
        void releaseResources() override;

        juce::AudioDeviceManager deviceManager;

    private:
        //==============================================================================
        void setAudioChannels(int numInputChannels, int numOutputChannels, const juce::XmlElement* const storedSettings = nullptr);

        juce::AudioSourcePlayer audioSourcePlayer;

        //==============================================================================
        anyMidi::ForwardFFT fft;
        anyMidi::MidiProcessor midiProc;

        juce::AudioSampleBuffer processingBuffer;

        juce::IIRFilter hiPassFilter;

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


        // JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioProcessor)
    };
}; // namespace anyMidi
