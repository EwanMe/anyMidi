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

std::vector<std::pair<int, double>> ForwardFFT::getHarmonics(const unsigned int& numPartials, const std::vector<double>& noteFreq)
{
    auto data = getFFTData();
    cleanUpBins(data);
    auto notes = mapBinsToNotes(noteFreq, data);
    return determineHarmonics(numPartials, notes);
}

void ForwardFFT::cleanUpBins(std::array<float, fftSize*2>& data)
{
    std::vector<int> lobes;
    for (int bin = 0; bin < getFFTSize(); ++bin)
    {
        // Clean up noise - acts like a gate.
        if (data[bin] < 1)
        {
            data[bin] = 0;
        }
        else
        {
            // Adds bin as part of a lobe when above threshold.
            lobes.push_back(bin);
        }
        
        // When bin is zero, we've moved past the lobe and it can be analyzed.
        if (lobes.size() > 0 && data[bin] == 0)
        {
            int centerLobe = std::floor((lobes.size() - 1) / 2);
            int ctrBin = lobes[centerLobe];
            for (int i : lobes)
            {
                // Adds all amplitudes to center bin.
                if (i != ctrBin)
                {  
                    data[ctrBin] += data[i];
                    data[i] = 0;
                }
            }
            lobes.clear();
        }
    }
}

std::vector<double> ForwardFFT::mapBinsToNotes(const std::vector<double>& noteFreq, std::array<float, fftSize * 2>& data)
{
    // Determines closest note to all bins in FFT and thus maps bins to their correct frequencies.
    // The amplitudes of each bin is added onto the notes amplitude.
    std::vector<double> noteAmps(noteFreq.size());
    int i = 0;
    for (int bin = 1; bin < getFFTSize(); ++bin)
    {
        double freq = (double)bin * sampleRate / (fftSize * 2);
        int note = findNearestNote(freq, noteFreq);
        noteAmps[note] += data[bin];
    }

    return noteAmps;
}

std::vector<std::pair<int, double>> ForwardFFT::determineHarmonics(const unsigned int& numPartials, std::vector<double>& data) const
{
    // Thanks to https://stackoverflow.com/questions/14902876/indices-of-the-k-largest-elements-in-an-unsorted-length-n-array/38391603#38391603
    // for inspiration for this algorithm.

    // Stores bin index and value for the n loudest partials.
    std::priority_queue<std::pair<double, int>, std::vector<std::pair<double, int>>, std::greater<std::pair<double, int>>> queue;

    // Puts the loudest bins into the priority queue, storing the amplitude and the index.
    for (int i = 0; i < data.size(); ++i)
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
    std::vector<std::pair<int, double>> harmonics;
    for (int i = 0; i < k; ++i)
    {
        // Creates pair of {frequenzy, amplitude}.
        harmonics.push_back(std::make_pair(queue.top().second, queue.top().first / fftSize));
        queue.pop();
    }

    // Sorts harmonics based on lowest frequency.
    std::sort(harmonics.begin(), harmonics.end(),
        [](std::pair<int, double> a, std::pair<int, double> b)
        {
            return a.first < b.first;
        }
    );

    return harmonics;
}

int ForwardFFT::findNearestNote(const double& target, const std::vector<double>& noteFrequencies) const
{
    unsigned int begin = 0;
    unsigned int end = noteFrequencies.size();

    // When frequency is below or above highest midi note.
    if (target <= noteFrequencies[begin])
    {
        return begin;
    }
    if (target >= noteFrequencies[end - 1])
    {
        return end - 1;
    }

    // Variation of binary search.
    unsigned int mid;
    while (end - begin > 1)
    {
        mid = begin + (end - begin) / 2;

        if (target == noteFrequencies[mid])
        {
            return mid;
        }
        if (target < noteFrequencies[mid])
        {
            end = mid;
        }
        else
        {
            begin = mid;
        }
    }

    if (std::abs(target - noteFrequencies[begin]) < std::abs(target - noteFrequencies[end]))
    {
        return begin;
    }
    else
    {
        return end;
    }
}
