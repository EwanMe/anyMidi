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
    static constexpr unsigned int fftOrder{ 11 };
    static constexpr unsigned int fftSize = 1 << fftOrder;

public:
    ForwardFFT(const double sampleRate);

    void pushNextSampleIntoFifo(float sample);
    std::shared_ptr<std::array<float, ForwardFFT::fftSize * 2>> getFFTData() const;
    int getFFTSize() const;
    float calcFundamentalFreq();

    bool nextFFTBlockReady = false;
    
private:
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;

    std::array<float, fftSize> fifo;
    std::array<float, fftSize * 2> fftData;
    int fifoIndex = 0;
    double sampleRate;   
};