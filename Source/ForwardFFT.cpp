/*
  ==============================================================================

    ForwardFFT.cpp
    Created: 3 Mar 2021 12:55:26pm
    Author:  Hallvard Jensen

  ==============================================================================
*/

#include "ForwardFFT.h"
using namespace anyMIDI;

ForwardFFT::ForwardFFT(const double sampleRate)
    : forwardFFT{ fftOrder },
    sampleRate{ sampleRate },
    // When initialising the windowing function, consider using fftSize + 1, ref. https://artandlogic.com/2019/11/making-spectrograms-in-juce/amp/
    window{ fftSize + 1, juce::dsp::WindowingFunction<float>::hann }
{

}

std::array<float, ForwardFFT::fftSize * 2> ForwardFFT::getFFTData() const
{
    return fftData;
}


int ForwardFFT::getFFTSize() const
{
    return fftSize;
}

void ForwardFFT::pushNextSampleIntoFifo(float sample)
{
    // When fifo contains enough data, flag is set to say next frame should be rendered.
    if (fifoIndex == fftSize)
    {
        if (!nextFFTBlockReady)
        {
            // Initializes fftData with zeroes.
            std::fill(fftData.begin(), fftData.end(), 0.0f);
            // Copies the data from the fifo into fftData.
            std::copy(fifo.begin(), fifo.end(), fftData.begin());
            // Sets flag.
            nextFFTBlockReady = true;

            // Perform windowing and forward FFT.
            window.multiplyWithWindowingTable(fftData.data(), fftSize);
            forwardFFT.performFrequencyOnlyForwardTransform(fftData.data());
            
            // Amplitude compensation for window function.
            juce::FloatVectorOperations::multiply(fftData.data(), windowCompensation, fftSize);
        }

        fifoIndex = 0;
    }

    fifo[fifoIndex++] = sample;
}

std::pair<double, double> ForwardFFT::calcFundamentalFreq() const
{
    double max{ 0 };
    unsigned int targetBin{ 0 }; // Location of fund. freq. will be stored here.
    auto data = getFFTData();

    // Finds fft bin with most energy.
    for (unsigned int i = 0; i < getFFTSize(); ++i)
    {
        if (max < data[i])
        {
            max = data[i];
            targetBin = i;
        }
    }

    // Calculates frequency from bin number and accesses amplitude at bin number.
    auto fundamental = std::make_pair((double)targetBin * sampleRate / fftSize, (double)(data[targetBin] / fftSize));
    return fundamental;
}
