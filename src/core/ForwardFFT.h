/**
 *
 *  @file      ForwardFFT.h
 *  @brief     Processing and spectral analysis of FFT data.
 *  @author    Hallvard Jensen
 *  @date      3 Mar 2021
 *  @copyright Hallvard Jensen, 2021. All right reserved.
 *
 */

#pragma once

#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>

namespace anyMidi {

class ForwardFFT {
private:
    /// Used as exponent of base 2 in FFT size.
    static constexpr size_t fftOrder{10};
    /// 2 to the power of FFT order.
    static constexpr size_t fftSize = 1UL << fftOrder;

    /// Signals whether the FIFO has been copied into the FFT array.
    bool nextFFTBlockReady_ = false;

public:
    /**
     *  @brief ForwardFFT object constructor
     *  @param sampleRate      - Audio sample rate to use for the FFT.
     *  @param windowingMethod - Windowing method to use for the FFT.
     */
    ForwardFFT(
        double sampleRate,
        juce::dsp::WindowingFunction<float>::WindowingMethod windowingMethod);

    static int getFFTSize();

    std::array<float, fftSize * 2> getFFTData() const;

    bool isNextFFTBlockReady() const { return nextFFTBlockReady_; }

    void setNextFFTBlockReady(const bool ready) { nextFFTBlockReady_ = ready; }

    int getWindowingFunction() const;

    void setWindowingFunction(const int &id);

    /**
     *  @brief  Used to initialize UI with possible windowing methods.
     *  @retval  - Available windowing methods as strings.
     */
    juce::Array<juce::String> getAvailableWindowingMethods() const;

    /**
     *  @brief Fills the FIFO with samples and initiates FFT on the sample when
     *         the FIFO is full.
     *  @param sample - The sample to be stored in the FIFO.
     */
    void pushNextSampleIntoFifo(float sample);

    /**
     *  @brief  Quick and dirty calculation of the fundamental frequency of the
     *          current FFT data array. Only consideres the loudest bin, and is
     *          therefore deprecated.
     *  @retval  - Pair of the fundamental frequency and its amplitude.
     */
    std::pair<double, double> calcFundamentalFreq() const;

    /**
     *  @brief  Determines the harmonic partials present in the current FFT
     * data.
     *  @param  numPartials - Number of partials to retrieve.
     *  @param  noteFreq    - Frequencies of MIDI note values, indexed by the
     * note value.
     *  @retval             - Pairs of frequency and amplitude for each of the
     *                        partials.
     */
    std::vector<std::pair<int, double>>
    getHarmonics(const unsigned int &numPartials,
                 const std::vector<double> &noteFreq) const;

    /**
     *  @brief Zeroes out all bins below a threshold. Lobes in the frequency
     *         spectrum are compressed into single bins.
     *  @param data - Bins of the FFT data.
     */
    static void cleanUpBins(std::array<float, fftSize * 2> &data);

    /**
     *  @brief  Maps all bins to the closest frequency corresponding to a note
     *          value to be able to determine harmonical relationships.
     *  @param  noteFreq - Sorted vector of note frequencies with their index
     *                     corresponding to the MIDI note value.
     *  @param  data     - Bins of the FFT data.
     *  @retval          - Amplitudes for all MIDI note values.
     */
    std::vector<double>
    mapBinsToNotes(const std::vector<double> &noteFreq,
                   const std::array<float, fftSize * 2UL> &data) const;

    /**
     *  @brief  Finds the note values with largest amplitudes, determining them
     * as harmonics of the signal.
     *  @param  numPartials - Number of partials to retrieve in the harmonic
     *                        spectrum.
     *  @param  amps        - Amplitudes for all MIDI note values.
     *  @retval             - Frequencies and amplitudes of the partials found,
     *                        sorted by frequencies in ascending order.
     */
    std::vector<std::pair<int, double>> static determineHarmonics(
        const unsigned int &numPartials, std::vector<double> &amps);

private:
    std::array<float, fftSize * 2UL> fftData_{0};
    std::array<float, fftSize> fifo_{0}; /// Next block to be loaded into FFT.
    int fifoIndex_ = 0;                  /// Iterator for FIFO.

    const double sampleRate_;

    juce::dsp::FFT forwardFFT_;
    juce::dsp::WindowingFunction<float> window_;
    juce::dsp::WindowingFunction<float>::WindowingMethod winMethod_;

    float windowCompensation_; /// Factor to compensate windowed FFT amplitudes
                               /// with.

    /// Mappings of windowing methods to amplitude compensation factor.
    const std::map<juce::dsp::WindowingFunction<float>::WindowingMethod, float>
        windowCompensations_{
            // Correction factor for triangular and blackman-harris not entered
            // These will not be put in the dropdown selection.
            {juce::dsp::WindowingFunction<float>::rectangular, 1.0},
            {juce::dsp::WindowingFunction<float>::hann, 2.0},
            {juce::dsp::WindowingFunction<float>::hamming, 1.85},
            {juce::dsp::WindowingFunction<float>::blackman, 2.8},
            {juce::dsp::WindowingFunction<float>::flatTop, 4.18},
            {juce::dsp::WindowingFunction<float>::kaiser, 2.49},
        };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ForwardFFT)
};

/**
 *  @brief  Estimates a frequency's closest approximation among a set of
 *          frequencies. By passing frequencies corresponding to the 127 MIDI
 *          notes, the function returns the nearest MIDI note value.
 *  @param  target          - Frequency to estimate.
 *  @param  noteFrequencies - Frequencies to search among.
 *  @retval                 - Index of note in vector closest to target
 *                            frequency.
 */
int findNearestNote(const double &target,
                    const std::vector<double> &noteFrequencies);

} // namespace anyMidi