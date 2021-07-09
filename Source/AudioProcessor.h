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
    //==============================================================================
    class AudioDeviceManagerRCO :   public juce::AudioDeviceManager,
                                    public juce::ReferenceCountedObject 
    {
    public:
        using Ptr = juce::ReferenceCountedObjectPtr<AudioDeviceManagerRCO>;
    };

    //==============================================================================
    class AudioProcessor :  public juce::AudioSource, 
                            public juce::ValueTree::Listener
    {
    public:
        //==============================================================================
        AudioProcessor(juce::ValueTree v);
        ~AudioProcessor() override;

        void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
        void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
        void releaseResources() override;

        void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;

        anyMidi::AudioDeviceManagerRCO::Ptr deviceManager;

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
        static constexpr unsigned int numOutputChannels{ 0 };

        //==============================================================================
        // Lookup array to determine Midi notes from frequencies.
        std::vector<double> noteFrequencies;
        static constexpr unsigned int tuning{ 440 }; // Can't be changed due to the MIDI protocol.

        void setNumPartials(int& n);

        // Determines values for Midi message based on FFT analysis.
        void calcNote();

        // Returns note value based on analysis of harmonics.
        std::pair<int, double> analyzeHarmonics();
        unsigned int numPartials{ 6 };


        juce::ValueTree tree;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioProcessor)
    };
}; // namespace anyMidi
