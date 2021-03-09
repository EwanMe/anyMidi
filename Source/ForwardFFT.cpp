/*
  ==============================================================================

    ForwardFFT.cpp
    Created: 3 Mar 2021 12:55:26pm
    Author:  Hallvard Jensen

  ==============================================================================
*/

#include "ForwardFFT.h"

ForwardFFT::ForwardFFT(const double sampleRate)
    : forwardFFT{ fftOrder },
    sampleRate{ sampleRate },
    // When initialising the windowing function, consider using fftSize + 1, ref. https://artandlogic.com/2019/11/making-spectrograms-in-juce/amp/
    window{ fftSize + 1, juce::dsp::WindowingFunction<float>::blackman }
{

}

std::shared_ptr<std::array<float, ForwardFFT::fftSize * 2>> ForwardFFT::getFFTData() const
{
    return std::make_shared<std::array<float, ForwardFFT::fftSize * 2>>(fftData);
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
            // Copies the data from the fifo into fftData.
            std::fill(fftData.begin(), fftData.end(), 0.0f);
            std::copy(fifo.begin(), fifo.end(), fftData.begin());
            nextFFTBlockReady = true;

            // Perform windowing and forward FFT.
            window.multiplyWithWindowingTable(fftData.data(), fftSize);
            forwardFFT.performFrequencyOnlyForwardTransform(fftData.data());
        }

        fifoIndex = 0;
    }

    fifo[fifoIndex++] = sample;
}

float ForwardFFT::calcFundamentalFreq()
{
    float max{ 0 };
    auto bin{ 0 };
    unsigned int targetIndex{ 0 };
    auto data = getFFTData();

    for (unsigned int i = 1; i < getFFTSize(); ++i)
    {

        if (max < data->at(i))
        {
            max = data->at(i);
            targetIndex = i;
        }
    }

    auto fundFreq = (float)targetIndex * sampleRate / fftSize;
    return fundFreq;
}
