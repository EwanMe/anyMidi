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
        ForwardFFT(const double sampleRate);

        void pushNextSampleIntoFifo(float sample);

        // Returns the FFT data array.
        std::array<float, fftSize * 2> getFFTData() const;
        int getFFTSize() const;

        // Calculates the fundamental frequency of the current FFT data array.
        std::pair<double, double> calcFundamentalFreq() const;

        // Fetches the number of harmonics specified, where the FFT bins are mapped to the provided note frequencies.
        // Returns pairs of {frequency, amplitude} for each harmonic.
        std::vector<std::pair<int, double>> getHarmonics(const unsigned int& numPartials, const std::vector<double>& noteFreq);
        std::array<float, fftSize*2> cleanUpLobes();

        // Takes in vector of frequencies corresponding to musical notes, and maps the bins in the FFT to these frequencies.
        std::vector<double> mapBinsToNotes(const std::vector<double>& noteFreq);

        // Finds the bins with largest amplitudes.
        std::vector<std::pair<int, double>> analyzeHarmonics(const unsigned int& numPartials, std::vector<double>& data) const;

        int findNearestNote(const double& target, const std::vector<double>& noteFrequencies) const;

        bool nextFFTBlockReady = false;

    private:
        juce::dsp::FFT forwardFFT;

        int fifoIndex = 0;
        std::array<float, fftSize> fifo;
        std::array<float, fftSize * 2> fftData;

        juce::dsp::WindowingFunction<float> window;
        static constexpr float windowCompensation{ 2.0 }; //2.2
        
        const double sampleRate;
    };
} // namespace anyMidi