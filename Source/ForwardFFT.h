/**
 *
 *  @file      ForwardFFT.h
 *  @brief     Processing and spectral analysis of FFT data.
 *  @author    Hallvard Jensen
 *  @date      3 Mar 2021 12:55:10pm
 *  @copyright © Hallvard Jensen, 2021. All right reserved.
 *
 */

#pragma once

#include <JuceHeader.h>

namespace anyMidi {

    class ForwardFFT 
    {

    private:
        static constexpr unsigned int fftOrder{ 10 }; /// Used as exponent of base 2 in FFT size.
        static constexpr unsigned int fftSize = 1 << fftOrder; /// 2 to the power of FFT order.


    public:
        bool nextFFTBlockReady = false; /// Signals whether the FIFO has been copied into the FFT array.


        /**
         *  @brief ForwardFFT object constructor
         *  @param sampleRate      - Audio sample rate to use for the FFT.
         *  @param windowingMethod - Windowing method to use for the FFT.
         */
        ForwardFFT(const double sampleRate, const juce::dsp::WindowingFunction<float>::WindowingMethod windowingMethod);


        int getFFTSize() const;
        
        std::array<float, fftSize * 2> getFFTData() const;
        
        int getWindowingFunction() const;
        
        void setWindowingFunction(const int &id);


        /**
         *  @brief  Used to initialize UI with possible windowing methods.
         *  @retval  - Available windowing methods as strings.
         */
        juce::Array<juce::String> getAvailableWindowingMethods() const;
        
        
        /**
         *  @brief Fills the FIFO with samples and initiates FFT on the sample when the FIFO is full.
         *  @param sample - The sample to be stored in the FIFO.
         */
        void pushNextSampleIntoFifo(float sample);

        
        /**
         *  @brief  Quick and dirty calculation of the fundamental frequency of the current FFT data array.
         *          Only consideres the loudest bin, and is therefore deprecated.
         *  @retval  - Pair of the fundamental frequency and its amplitude.
         */
        std::pair<double, double> calcFundamentalFreq() const;

        
        /**
         *  @brief  Determines the harmonic partials present in the current FFT data.
         *  @param  numPartials - Number of partials to retrieve.
         *  @param  noteFreq    - Frequencies of MIDI note values, indexed by the note value.
         *  @retval             - Pairs of frequency and amplitude for each of the partials.
         */
        std::vector<std::pair<int, double>> getHarmonics(const unsigned int& numPartials, const std::vector<double>& noteFreq);
        
        
        /**
         *  @brief Zeroes out all bins below a threshold. Lobes in the frequency spectrum are compressed into single bins.
         *  @param data - Bins of the FFT data.
         */
        void cleanUpBins(std::array<float, fftSize * 2>& data);

        
        /**
         *  @brief  Maps all bins to the closest frequency corresponding to a note value 
                    to be able to determine harmonical relationships.
         *  @param  noteFreq - Sorted vector of note frequencies with their index corresponding to the MIDI note value.
         *  @param  data     - Bins of the FFT data.
         *  @retval          - Amplitudes for all MIDI note values.
         */
        std::vector<double> mapBinsToNotes(const std::vector<double>& noteFreq, const std::array<float, fftSize * 2>& data);

        
        /**
         *  @brief  Finds the note values with largest amplitudes, determining them as harmonics of the signal.
         *  @param  numPartials - Number of partials to retrieve in the harmonic spectrum.
         *  @param  amps        - Amplitudes for all MIDI note values.
         *  @retval             - Frequencies and amplitudes of the partials found, sorted by frequencies in ascending order.
         */
        std::vector<std::pair<int, double>> determineHarmonics(const unsigned int& numPartials, std::vector<double>& amps) const;


    private:
        
        std::array<float, fftSize * 2> fftData;
        std::array<float, fftSize> fifo; /// Next block to be loaded into FFT.
        int fifoIndex = 0; /// Iterator for FIFO.
        
        const double sampleRate;
        
        juce::dsp::FFT forwardFFT;
        juce::dsp::WindowingFunction<float> window;
        juce::dsp::WindowingFunction<float>::WindowingMethod winMethod;
        
        float windowCompensation; /// Factor to compensate windowed FFT amplitudes with.
        
        /// Mappings of windowing methods to amplitude compensation factor.
        const std::map<juce::dsp::WindowingFunction<float>::WindowingMethod, float> windowCompensations
        {
            // Correction factor for triangular and blackman-harris not entered
            // These will not be put in the dropdown selection.
            { juce::dsp::WindowingFunction<float>::rectangular, 1.0 },
            { juce::dsp::WindowingFunction<float>::hann, 2.0 },
            { juce::dsp::WindowingFunction<float>::hamming, 1.85 },
            { juce::dsp::WindowingFunction<float>::blackman, 2.8 },
            { juce::dsp::WindowingFunction<float>::flatTop, 4.18 },
            { juce::dsp::WindowingFunction<float>::kaiser, 2.49 },
        };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ForwardFFT)
    };


    /**
     *  @brief  Estimates a frequency's closest approximation among a set of frequencies.
     *          By passing frequencies corresponding to the 127 MIDI notes, the function returns the nearest MIDI note value.
     *  @param  target          - Frequency to estimate.
     *  @param  noteFrequencies - Frequencies to search among.
     *  @retval                 - Index of note in vector closest to target frequency.
     */
    int findNearestNote(const double& target, const std::vector<double>& noteFrequencies);

} // namespace anyMidi