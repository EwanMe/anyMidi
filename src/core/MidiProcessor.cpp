/**
 *
 *  @file      MidiProcessor.cpp
 *  @author    Hallvard Jensen
 *  @date      8 Apr 2021
 *  @copyright Hallvard Jensen, 2021. All right reserved.
 *
 */

#include "MidiProcessor.h"

anyMidi::MidiProcessor::MidiProcessor(const unsigned int &sampleRate,
                                      const double &startTime)
    : sampleRate_{sampleRate}, startTime_{startTime} {}

void anyMidi::MidiProcessor::setMidiOutput(juce::MidiOutput *output) {
    midiOut_ = output;
}

auto anyMidi::MidiProcessor::getAttackThreshold() const -> double {
    return attackThreshold_;
}

auto anyMidi::MidiProcessor::getReleaseThreshold() const -> double {
    return releaseThreshold_;
}

void anyMidi::MidiProcessor::setAttackThreshold(double &t) {
    attackThreshold_ = t;
}

void anyMidi::MidiProcessor::setReleaseThreshold(double &t) {
    releaseThreshold_ = t;
}

auto anyMidi::MidiProcessor::determineNoteValue(
    const int &note, const double &amp,
    std::vector<std::pair<int, bool>> &noteValues) -> bool {
    // Ensures that notes are within midi range.
    constexpr std::pair<int, int> kMidiRange{0, 128};
    if (note >= kMidiRange.first && note < kMidiRange.second) {
        // When there's no note currently playing, and the note
        // surpasses the threshold.
        if (!midiNoteCurrentlyOn_ && amp > attackThreshold_) {
            noteValues.emplace_back(note, true); // Note on
            midiNoteCurrentlyOn_ = true;
            lastNote_ = note;
            lastAmp_ = amp;
            return true;
        }
        // When another note is currently playing.
        if (midiNoteCurrentlyOn_) {
            // When new note is different from the last note.
            if (note != lastNote_) {
                // When new, different note surpasses threshold.
                // Last note is turned off before new note is turned on.
                if (amp > attackThreshold_) {
                    noteValues.emplace_back(lastNote_, false); // Note off
                    noteValues.emplace_back(note, true);       // Note on
                    midiNoteCurrentlyOn_ = true;
                    lastNote_ = note;
                    lastAmp_ = amp;
                    return true;
                }
                return false;
            }
            // When new note is the same as last note,
            // it has to be sufficiently louder to retrigger.
            if (amp > lastAmp_ * 3) {
                if (amp > attackThreshold_) {
                    noteValues.emplace_back(lastNote_, false); // Note off
                    noteValues.emplace_back(note, true);       // Note on
                    midiNoteCurrentlyOn_ = true;
                    lastNote_ = note;
                    lastAmp_ = amp;
                    return true;
                }
                return false;
            }
            // When new note is not different, nor louder than last
            // it's probably a releasing note and we check if it has rung out.
            if (amp < releaseThreshold_) {
                noteValues.emplace_back(lastNote_, false); // Note off
                midiNoteCurrentlyOn_ = false;
                return true;
            }
            lastAmp_ = amp;
        }
    }
    return false;
}

void anyMidi::MidiProcessor::createMidiMsg(const int &noteNum,
                                           const juce::uint8 &velocity,
                                           const bool noteOn) {
    juce::MidiMessage midiMessage;

    // JUCE is one octave off for some reason.
    constexpr int juceOctaveOffset = 12;
    const int scaledNoteNum = noteNum + juceOctaveOffset;
    // Bounds represent note range of a typical guitar
    constexpr int noteLowerBound{40};
    constexpr int noteUpperBound{90};
    if (scaledNoteNum >= noteLowerBound && scaledNoteNum < noteUpperBound) {
        if (noteOn) {
            midiMessage = juce::MidiMessage::noteOn(midiChannel, scaledNoteNum,
                                                    velocity);
        } else {
            midiMessage =
                juce::MidiMessage::noteOff(midiChannel, scaledNoteNum);
        }

        constexpr double secondScale = 0.001;
        midiMessage.setTimeStamp(
            juce::Time::getMillisecondCounter() * secondScale - startTime_);
        addMessageToBuffer(midiMessage);
    }
}

void anyMidi::MidiProcessor::addMessageToBuffer(
    const juce::MidiMessage &message) {
    const double timestamp = message.getTimeStamp();
    const int sampleNumber = static_cast<int>(timestamp * sampleRate_);

    midiBuffer_.addEvent(message, sampleNumber);
}

void anyMidi::MidiProcessor::pushBufferToOutput() {
    if (midiOut_) {
        midiOut_->startBackgroundThread();
    }

    if (midiOut_ != nullptr) {
        midiOut_->sendBlockOfMessagesNow(midiBuffer_);
        midiBuffer_.clear();
    }
}

void anyMidi::MidiProcessor::turnOffAllMessages() {
    if (midiOut_) {
        midiOut_->sendMessageNow(juce::MidiMessage::allNotesOff(midiChannel));
    }
}
