/**
 *
 *  @file      AudioProcessor.h
 *  @brief     Audio device interaction and aggregation of MIDI and FFT.
 *  @author    Hallvard Jensen
 *  @date      22.06.2021
 *  @copyright Hallvard Jensen, 2021. All right reserved.
 *
 */

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>

#include "ForwardFFT.h"
#include "MidiProcessor.h"

namespace anyMidi {

/**
 *
 *  @class   AudioDeviceManagerRCO
 *  @brief   Wrapper for the JUCE AudioDriverManager class, making it a
 *           reference counted object This allows for passing it through the
 *           juce::ValueTree class.
 *
 */
class AudioDeviceManagerRCO : public juce::AudioDeviceManager,
                              public juce::ReferenceCountedObject {
public:
    using Ptr = juce::ReferenceCountedObjectPtr<AudioDeviceManagerRCO>;
};

/**
 *
 *  @class   AudioProcessor
 *  @brief   Main audio processing class interacting with the sound card.
 *
 */
class AudioProcessor : public juce::AudioSource,
                       public juce::ValueTree::Listener {
public:
    explicit AudioProcessor(double sampleRate, const juce::ValueTree &v);

    ~AudioProcessor() override;

    /**
     *  @brief Initializes audio processor. Called upon application start.
     */
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;

    /**
     *  @brief Handles and processes incoming samples from audio source.
     *  @param bufferToFill - The audio buffer of the application.
     */
    void getNextAudioBlock(
        const juce::AudioSourceChannelInfo &bufferToFill) override;

    /**
     *  @brief Called upon application quit.
     */
    void releaseResources() override;

    /**
     *  @brief Callback function triggered by changes in ValueTree.
     */
    void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                  const juce::Identifier &property) override;

private:
    juce::AudioSourcePlayer audioSourcePlayer_;
    anyMidi::ForwardFFT fft_;
    anyMidi::MidiProcessor midiProc_;
    anyMidi::AudioDeviceManagerRCO::Ptr deviceManager_;
    juce::AudioSampleBuffer processingBuffer_;
    juce::IIRFilter hiPassFilter_;

    static constexpr double lowFilterFreq{75.0}; // E5 on guitar ~82 Hz
    static constexpr double highFilterFreq{24000.0};

    static constexpr unsigned int numInputChannels{1};
    static constexpr unsigned int numOutputChannels{0};

    /// Optimized number of partials for the BSc project
    static constexpr int defaultNumPartials{6};

    int numPartials_{defaultNumPartials};
    std::vector<double> noteFrequencies_; /// Lookup array to determine Midi
                                          /// notes from frequencies.

    juce::ValueTree tree_; /// Container for data shared with the GUI.

    /**
     *  @brief Updates number of partials for harmonic analysis.
     *  @param n - Number of partials to consider during the harmonic analysis.
     */
    void setNumPartials(int &n);

    /**
     *  @brief Creates a MIDI message with note value and amplitude retrieved
     * from FFT analysis.
     */
    void calcNote();

    /**
     *  @brief  Determines a signals note value by doing a weighted analysis on
     *          the harmonical spectrum.
     *  @retval  - A pair of the estimated note value with its summed signal
     *             amplitude.
     */
    std::pair<int, double> analyzeHarmonics();

    /**
     *  @brief Initializes audio device manager's audio channels.
     *  @param numInputChannels  - Number of inputs.
     *  @param numOutputChannels - Number of outputs.
     *  @param storedSettings    - Optional XML element containing settings to
     *                             initialize device manager with.
     */
    void
    setAudioChannels(int numInputChannels, int numOutputChannels,
                     const juce::XmlElement *const storedSettings = nullptr);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioProcessor)
};
}; // namespace anyMidi
