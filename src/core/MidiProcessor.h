/**
 *
 *  @file      MidiProcessor.h
 *  @brief     Managing MIDI device interactions.
 *  @author    Hallvard Jensen
 *  @date      8 Apr 2021
 *  @copyright Hallvard Jensen, 2021. All right reserved.
 *
 */

#pragma once

#include <JuceHeader.h>

namespace anyMidi {

/**
 *
 *  @class   MidiProcessor
 *  @brief   Manages turning on and off MIDI notes.
 *
 */
class MidiProcessor {
public:
    MidiProcessor(const unsigned int &sampleRate, const double &startTime);

    void setMidiOutput(juce::MidiOutput *output);

    double getAttackThreshold() const;
    double getReleaseThreshold() const;

    void setAttackThreshold(double &t);
    void setReleaseThreshold(double &t);

    /**
     *  @brief Empties MIDI buffer and clears it.
     */
    void pushBufferToOutput();

    /**
     *  @brief Catch-all function to make sure no MIDI messages are left turned
     *         on.
     */
    void turnOffAllMessages();

    /**
     *  @brief  Determines if there is need for new MIDI note and decides wether
                to add a new note, and wether to turn off the last MIDI note if
                one is still playing.
     *  @param  note       - Midi note value.
     *  @param  amp        - Amplitude of the midi note.
     *  @param  noteValues - A return vector in which determined midi notes are
     *                       placed.
     *  @retval            - Flag signaling if there is need to create new midi
     *                       messages.
     */
    bool determineNoteValue(const int &note, const double &amp,
                            std::vector<std::pair<int, bool>> &noteValues);

    /**
     *  @brief Creates a new MIDI message and pushes it to the buffer.
     *  @param noteNum  - MIDI note number.
     *  @param velocity - MIDI note velocity.
     *  @param noteOn   - Flag indicating if note is to be turned on or off.
     */
    void createMidiMsg(const int &noteNum, const juce::uint8 &velocity,
                       const bool noteOn);

    /**
     *  @brief Adds Midi message into buffer to be retrieved upon callback.
     *  @param message - The MIDI message to be added.
     */
    void addMessageToBuffer(const juce::MidiMessage &message);

private:
    juce::MidiBuffer midiBuffer_;
    juce::MidiOutput *midiOut_{nullptr};

    static constexpr int midiChannel{10};
    const unsigned int sampleRate_;

    /// Flag indicating if a MIDI note on has been sent without being turned off
    /// yet.
    bool midiNoteCurrentlyOn_{false};
    /// Previous MIDI note, used to determine note change.
    int lastNote_{-1};
    /// Previous amplitude of note, used to determine retrigger or ring out.
    double lastAmp_{0.0};

    double attackThreshold_{0.1};
    double releaseThreshold_{0.001};

    /// Audio app start time. Used to determine Midi message timestamp.
    const double startTime_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiProcessor)
};
} // namespace anyMidi