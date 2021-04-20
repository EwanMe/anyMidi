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
    auto correctedBins = mapBinsToNotes(noteFreq);
    return analyzeHarmonics(numPartials, correctedBins);
}

std::array<float, ForwardFFT::fftSize*2> ForwardFFT::cleanUpLobes()
{
    auto data = getFFTData();

    std::vector<int> lobe;
    for (int i = 0; i < getFFTSize(); ++i)
    {
        
    }
    return data;
}

std::vector<double> ForwardFFT::mapBinsToNotes(const std::vector<double>& noteFreq)
{
    auto data = getFFTData();
    std::vector<double> noteAmps(noteFreq.size());

    int i = 0;
    for (int bin = 1; bin < getFFTSize(); ++bin)
    {
        double freq = (double)bin * sampleRate / (fftSize * 2);
        int note = findNearestNote(freq, noteFreq);
        noteAmps[note] += data[bin];
        /*while (i < noteFreq.size()-1)
        {
            if (std::abs(freq - noteFreq[i]) > std::abs(freq - noteFreq[i+1]))
            {
                i++;
                continue;
            }
            noteAmps[i++] += data[bin];
            break;
        }*/
    }

    std::vector<int> lobes;
    std::vector<double> reestimatedNoteAmps(noteFreq.size());
    for (int i = 0; i < noteAmps.size(); ++i)
    {
        if (noteAmps[i] > 0)
        {
            lobes.push_back(i);
        }
        if (lobes.size() > 0 && noteAmps[i] == 0.0)
        {
            int centerLobe = std::floor((lobes.size()-1) / 2);
            int ctrNoteVal = lobes[centerLobe];
            for (int l : lobes)
            {
                reestimatedNoteAmps[ctrNoteVal] += noteAmps[l];
            }
            lobes.clear();
        }
    }

    return reestimatedNoteAmps;
}

std::vector<std::pair<int, double>> ForwardFFT::analyzeHarmonics(const unsigned int& numPartials, std::vector<double>& data) const
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

/* ---
* Finne de n sterkeste bins i en fft.
* Kalkulere amplituder til overtoner utfra liste med gitte frekvenser
* Sjekke hvilken tone som spilles utfra vektet vurdering av overtoner, sterkeste frekvens bestemmer tone-oktav, men ikke nødvendigvis tone.
* Generer MIDI-note utfra kalkulert tone og summen av alle ampitudeverdier i overtonespekter.
--- */

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
