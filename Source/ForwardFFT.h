/*
  ==============================================================================

    ForwardFFT.h
    Created: 3 Mar 2021 12:55:10pm
    Author:  Hallvard Jensen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>


class ForwardFFT {
private:
    static constexpr unsigned int fftOrder{ 10 };
    static constexpr unsigned int fftSize = 1 << fftOrder;

public:
    ForwardFFT(const double sampleRate);

    void pushNextSampleIntoFifo(float sample);

    // Returns pointer to the FFT data array.
    std::array<float, ForwardFFT::fftSize * 2> getFFTData() const;
    int getFFTSize() const;

    // Calculates the fundamental frequency of the current FFT data array.
    std::pair<double, double> calcFundamentalFreq() const;

    bool nextFFTBlockReady = false;
    
private:
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;
    static constexpr float windowCompensation{ 2.0 };

    int fifoIndex = 0;
    std::array<float, fftSize> fifo;
    std::array<float, fftSize * 2> fftData;
    
    const double sampleRate;   
};