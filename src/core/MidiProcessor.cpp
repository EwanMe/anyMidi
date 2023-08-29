/**
 *
 *  @file      MidiProcessor.cpp
 *  @author    Hallvard Jensen
 *  @date      8 Apr 2021
 *  @copyright Hallvard Jensen, 2021. All right reserved.
 *
 */

#include "MidiProcessor.h"

using namespace anyMidi;

MidiProcessor::MidiProcessor(const unsigned int &sampleRate,
                             const double &startTime)
    : sampleRate{sampleRate}, startTime{startTime}, midiOut{nullptr} {}

void MidiProcessor::setMidiOutput(juce::MidiOutput *output) {
    midiOut = output;
}

double MidiProcessor::getAttackThreshold() const { return attackThreshold; }

double MidiProcessor::getReleaseThreshold() const { return releaseThreshold; }

void MidiProcessor::setAttackThreshold(double &t) { attackThreshold = t; }

void MidiProcessor::setReleaseThreshold(double &t) { releaseThreshold = t; }

bool MidiProcessor::determineNoteValue(
    const unsigned int &note, const double &amp,
    std::vector<std::pair<int, bool>> &noteValues) {
    // Ensures that notes are within midi range.
    if (note >= 0 && note < 128) {
        // When there's no note currently playing, and the note
        // surpasses the threshold.
        if (!midiNoteCurrentlyOn && amp > attackThreshold) {
            noteValues.push_back(std::make_pair(note, true)); // Note on
            midiNoteCurrentlyOn = true;
            lastNote = note;
            lastAmp = amp;
            return true;
        }
        // When another note is currently playing.
        if (midiNoteCurrentlyOn) {
            // When new note is different from the last note.
            if (note != lastNote) {
                // When new, different note surpasses threshold.
                // Last note is turned off before new note is turned on.
                if (amp > attackThreshold) {
                    noteValues.push_back(
                        std::make_pair(lastNote, false)); // Note off
                    noteValues.push_back(std::make_pair(note, true)); // Note on
                    midiNoteCurrentlyOn = true;
                    lastNote = note;
                    lastAmp = amp;
                    return true;
                }
                return false;
            }
            // When new note is the same as last note,
            // it has to be sufficiently louder to retrigger.
            if (amp > lastAmp * 3) {
                if (amp > attackThreshold) {
                    noteValues.push_back(
                        std::make_pair(lastNote, false)); // Note off
                    noteValues.push_back(std::make_pair(note, true)); // Note on
                    midiNoteCurrentlyOn = true;
                    lastNote = note;
                    lastAmp = amp;
                    return true;
                }
                return false;
            }
            // When new note is not different, nor louder than last
            // it's probably a releasing note and we check if it has rung out.
            if (amp < releaseThreshold) {
                noteValues.push_back(
                    std::make_pair(lastNote, false)); // Note off
                midiNoteCurrentlyOn = false;
                return true;
            }
            lastAmp = amp;
        }
    }
    return false;
}

void MidiProcessor::createMidiMsg(const unsigned int &noteNum,
                                  const juce::uint8 &velocity,
                                  const bool noteOn) {
    juce::MidiMessage midiMessage;
    unsigned int scaledNoteNum =
        noteNum + 12; // Juce is one octave off for some reason.
    if (scaledNoteNum >= 40 && scaledNoteNum < 90) {
        if (noteOn) {
            midiMessage =
                juce::MidiMessage::noteOn(midiChannel, scaledNoteNum, velocity);
        } else {
            midiMessage =
                juce::MidiMessage::noteOff(midiChannel, scaledNoteNum);
        }
        midiMessage.setTimeStamp(juce::Time::getMillisecondCounter() * 0.001 -
                                 startTime);
        addMessageToBuffer(midiMessage);
    }
}

void MidiProcessor::addMessageToBuffer(const juce::MidiMessage &message) {
    double timestamp = message.getTimeStamp();
    int sampleNumber = (int)(timestamp * sampleRate);

    midiBuffer.addEvent(message, sampleNumber);
}

void MidiProcessor::pushBufferToOutput() {
    if (midiOut) {
        midiOut->startBackgroundThread();
    }

    if (midiOut != nullptr) {
        midiOut->sendBlockOfMessagesNow(midiBuffer);
        midiBuffer.clear();
    }
}

void MidiProcessor::turnOffAllMessages() {
    if (midiOut) {
        midiOut->sendMessageNow(juce::MidiMessage::allNotesOff(midiChannel));
    }
}
