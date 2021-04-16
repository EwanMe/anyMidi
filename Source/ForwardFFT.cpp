/*
  ==============================================================================

    ForwardFFT.cpp
    Created: 3 Mar 2021 12:55:26pm
    Author:  Hallvard Jensen

  ==============================================================================
*/

#include "ForwardFFT.h"
using namespace anyMidi;

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
    auto fundamental = std::make_pair((double)targetBin * sampleRate / (fftSize * 2), (double)(data[targetBin] / fftSize));
    return fundamental;
}

std::vector<std::pair<double, double>> ForwardFFT::getHarmonics(const unsigned int& numPartials, const std::vector<double>& noteFreq)
{
    //auto correctedBins = mapBinsToFrequencies(noteFreq);
    std::array<double, fftSize> data{};
    auto fft = getFFTData();
    for (int i = 0; i < fftSize; ++i)
    {
        data[i] = fft[i];
    }
    return calculateHarmonics(numPartials, data);
}

std::array<double, ForwardFFT::fftSize> ForwardFFT::mapBinsToFrequencies(const std::vector<double>& noteFreq)
{
    auto data = getFFTData();
    std::array<double, fftSize> correctedBins{};

    double freqInterval = sampleRate / (getFFTSize() * 2);

    unsigned int f = 1; // No need to include bin no. 0 (0 Hz).
    // Iterates through each frequency in input vector.
    for (unsigned int i = 0; i < noteFreq.size(); ++i)
    {
        // Maps all bins in vicinity of current frequency to this frequency.
        while (f < getFFTSize())
        {
            // Calcuates the actual frequency value.
            double freq = (double)f * sampleRate / (fftSize * 2);

            // Amplitude of bin is added to current note frequency when
            // bin has lower freq than note, since freqs closer to below note should have
            // been handled in last iteration or by the base case

            
            if (freq < noteFreq[i] || i == noteFreq.size()-1)
            {
                correctedBins[i] += data[f++];
            }
            else if (std::abs(freq - noteFreq[i]) < std::abs(freq - noteFreq[i+1]))
            {
                correctedBins[i] += data[f++];
            }
            else
            {
                break;
            }
        }
    }
    return correctedBins;
}

std::vector<std::pair<double, double>> ForwardFFT::calculateHarmonics(const unsigned int& numPartials, std::array<double, ForwardFFT::fftSize> data)
{
    // Thanks to https://stackoverflow.com/questions/14902876/indices-of-the-k-largest-elements-in-an-unsorted-length-n-array/38391603#38391603
    // for inspiration for this algorithm.

    // Stores bin index and value for the n loudest partials.
    std::priority_queue<std::pair<double, int>, std::vector<std::pair<double, int>>, std::greater<std::pair<double, int>>> queue;

    // Puts the loudest bins into the priority queue, storing the amplitude and the index.
    for (int i = 0; i < getFFTSize(); ++i)
    {
        if (queue.size() < numPartials)
        {
            queue.push(std::pair<double, int>{data[i], i});
        }
        else if (queue.top().first < data[i])
        {
            queue.pop();
            queue.push(std::pair<double, int>{data[i], i});
        }
    }

    // Transforms the priority queue into a vector.
    int k = queue.size();
    std::vector<std::pair<double, double>> harmonics;
    for (int i = 0; i < k; ++i)
    {
        // Creates pair of {frequenzy, amplitude}.
        harmonics.push_back(std::make_pair((double)queue.top().second * sampleRate / (double)(fftSize * 2), queue.top().first / fftSize));
        queue.pop();
    }

    // Sorts harmonics based on lowest frequency.
    std::sort(harmonics.begin(), harmonics.end(),
        [](std::pair<double, double> a, std::pair<double, double> b)
        {
            return a.first < b.first;
        }
    );

    return harmonics;
}

/* ---
* Finne de n sterkeste bins i en fft.
* Kalkulere amplituder til overtoner utfra liste med gitte frekvenser
* Sjekke hvilken tone som spilles utfra vektet vurdering av overtoner, sterkeste frekvens bestemmer tone-oktav, men ikke nødvendigvis tone.
* Generer MIDI-note utfra kalkulert tone og summen av alle ampitudeverdier i overtonespekter.
--- */
