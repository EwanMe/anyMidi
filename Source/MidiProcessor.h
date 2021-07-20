/*
  ==============================================================================

    MidiProcessor.h
    Created: 8 Apr 2021 3:10:50pm
    Author:  Hallvard Jensen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace anyMidi {
    class MidiProcessor {
    public:
        MidiProcessor(const unsigned int& sampleRate, const double& startTime);
        
        void setMidiOutput(juce::MidiOutput* output);
        double getAttackThreshold() const;
        double getReleaseThreshold() const;
        void setAttackThreshold(double& t);
        void setReleaseThreshold(double& t);
        
        void pushBufferToOutput();
        void turnOffAllMessages();

        // Determines if there is need for new midi note and alters the noteOn input
        // such that it states wheter note on or note off message is to be created.
        bool determineNoteValue(const unsigned int& note, const double& amp, std::vector<std::pair<int, bool>>& noteValues);

        // Creates MIDI message based on input note number and velocity.
        void createMidiMsg(const unsigned int& noteNum, const juce::uint8& velocity, const bool noteOn);

        // Adds Midi message into buffer to be retrieved upon callback.
        void addMessageToBuffer(const juce::MidiMessage& message);

    private:
        juce::MidiBuffer midiBuffer;
        juce::MidiOutput* midiOut;
        static constexpr int midiChannel{ 10 };
        const unsigned int sampleRate;
        int previoudSampleNum{ 0 };

        bool midiNoteCurrentlyOn{ false };
        int lastNote{ -1 };
        double lastAmp{ 0.0 };

        double attackThreshold{ 0.1 };
        double releaseThreshold{ 0.001 };

        // Audio app start time. Used to determine Midi message timestamp.
        const double startTime;
    };
} // namespace anyMidi