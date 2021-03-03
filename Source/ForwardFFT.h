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
    ForwardFFT();

    void pushNextSampleIntoFifo(float sample);
    std::array<float, fftSize*2> getFFTData() const;
    int getFFTSize() const;

    bool nextFFTBlockReady = false;
    
private:
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;

    std::array<float, fftSize> fifo;
    std::array<float, fftSize * 2> fftData;
    int fifoIndex = 0;
};