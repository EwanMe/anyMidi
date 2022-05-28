/*
  ==============================================================================

    ForwardFFT.h
    Created: 3 Mar 2021 12:55:10pm
    Author:  Hallvard Jensen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace anyMidi {

    class ForwardFFT {
    private:
        static constexpr unsigned int fftOrder{ 10 };
        static constexpr unsigned int fftSize = 1 << fftOrder;

    public:
        ForwardFFT(const double sampleRate, const juce::dsp::WindowingFunction<float>::WindowingMethod windowingMethod);

        void pushNextSampleIntoFifo(float sample);

        // Returns the FFT data array.
        std::array<float, fftSize * 2> getFFTData() const;
        int getFFTSize() const;

        int getWindowingFunction() const;
        juce::Array<juce::String> getAvailableWindowingMethods() const;
        void setWindowingFunction(const int &id);


        // Calculates the fundamental frequency of the current FFT data array.
        std::pair<double, double> calcFundamentalFreq() const;

        // Fetches the number of harmonics specified, where the FFT bins are mapped to the provided note frequencies.
        // Returns pairs of {frequency, amplitude} for each harmonic.
        std::vector<std::pair<int, double>> getHarmonics(const unsigned int& numPartials, const std::vector<double>& noteFreq);
        
        // Zeroes out bins below threshold. Compresses lobes in FFT into single bins.
        void cleanUpBins(std::array<float, fftSize * 2>& data);

        // Takes in vector of frequencies corresponding to musical notes, and maps the bins in the FFT to these frequencies.
        std::vector<double> mapBinsToNotes(const std::vector<double>& noteFreq, std::array<float, fftSize * 2>& data);

        // Finds the bins with largest amplitudes.
        std::vector<std::pair<int, double>> determineHarmonics(const unsigned int& numPartials, std::vector<double>& amps) const;

        bool nextFFTBlockReady = false;

    private:
        juce::dsp::FFT forwardFFT;

        int fifoIndex = 0;
        std::array<float, fftSize> fifo;
        std::array<float, fftSize * 2> fftData;

        juce::dsp::WindowingFunction<float> window;
        juce::dsp::WindowingFunction<float>::WindowingMethod winMethod;
        float windowCompensation;
        
        const double sampleRate;

        const std::map<juce::dsp::WindowingFunction<float>::WindowingMethod, float> windowCompensations
        {
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

    // Finds nearest note among the provided note frequencies.
    int findNearestNote(const double& target, const std::vector<double>& noteFrequencies);
} // namespace anyMidi