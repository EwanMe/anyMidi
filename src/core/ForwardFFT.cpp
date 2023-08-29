/**
 *
 *  @file      ForwardFFT.cpp
 *  @author    Hallvard Jensen
 *  @date      3 Mar 2021
 *  @copyright Hallvard Jensen, 2021. All right reserved.
 *
 */

#include "ForwardFFT.h"

#include "../util/Globals.h"

using namespace anyMidi;

ForwardFFT::ForwardFFT(
    const double sampleRate,
    const juce::dsp::WindowingFunction<float>::WindowingMethod windowingMethod)
    : forwardFFT{fftOrder}, sampleRate{sampleRate},
      // When initialising the windowing function, consider using fftSize + 1,
      // ref. https://artandlogic.com/2019/11/making-spectrograms-in-juce/amp/
      window{fftSize + 1, windowingMethod}, winMethod{windowingMethod} {
    fftData.fill(0.0);
    fifo.fill(0.0);
    windowCompensation = windowCompensations.at(windowingMethod);
}

std::array<float, ForwardFFT::fftSize * 2> ForwardFFT::getFFTData() const {
    return fftData;
}

int ForwardFFT::getFFTSize() const { return fftSize; }

void ForwardFFT::setWindowingFunction(const int &id) {
    auto winMethod = juce::dsp::WindowingFunction<float>::WindowingMethod(id);

    this->winMethod = winMethod;
    this->windowCompensation = windowCompensations.at(winMethod);
    this->window.fillWindowingTables(fftSize + 1, winMethod);
}

juce::Array<juce::String> ForwardFFT::getAvailableWindowingMethods() const {
    juce::Array<juce::String> windowStrings;
    windowStrings.resize(juce::dsp::WindowingFunction<
                         float>::WindowingMethod::numWindowingMethods);
    windowStrings.fill("");

    // Fill array with existing window method enums.
    for (auto &w : windowCompensations) {
        juce::dsp::WindowingFunction<float>::WindowingMethod method =
            juce::dsp::WindowingFunction<
                float>::WindowingMethod::numWindowingMethods;
        switch (w.first) {
        case juce::dsp::WindowingFunction<float>::rectangular:
            method = juce::dsp::WindowingFunction<float>::rectangular;
            break;
        case juce::dsp::WindowingFunction<float>::triangular:
            method = juce::dsp::WindowingFunction<float>::triangular;
            break;
        case juce::dsp::WindowingFunction<float>::hann:
            method = juce::dsp::WindowingFunction<float>::hann;
            break;
        case juce::dsp::WindowingFunction<float>::hamming:
            method = juce::dsp::WindowingFunction<float>::hamming;
            break;
        case juce::dsp::WindowingFunction<float>::blackman:
            method = juce::dsp::WindowingFunction<float>::blackman;
            break;
        case juce::dsp::WindowingFunction<float>::blackmanHarris:
            method = juce::dsp::WindowingFunction<float>::blackmanHarris;
            break;
        case juce::dsp::WindowingFunction<float>::flatTop:
            method = juce::dsp::WindowingFunction<float>::flatTop;
            break;
        case juce::dsp::WindowingFunction<float>::kaiser:
            method = juce::dsp::WindowingFunction<float>::kaiser;
            break;
        default:
            break;
        }

        if (method < juce::dsp::WindowingFunction<float>::numWindowingMethods) {
            windowStrings.insert(
                method,
                juce::dsp::WindowingFunction<float>::getWindowingMethodName(
                    method));
        }
    }

    return windowStrings;
}

void ForwardFFT::pushNextSampleIntoFifo(float sample) {
    // When fifo contains enough data, flag is set to say next frame should be
    // rendered.
    if (fifoIndex == fftSize) {
        if (!nextFFTBlockReady) {
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
            juce::FloatVectorOperationsBase<float, size_t>::multiply(
                fftData.data(), windowCompensation, fftSize);
        }

        fifoIndex = 0;
    }

    fifo[fifoIndex++] = sample;
}

std::pair<double, double> ForwardFFT::calcFundamentalFreq() const {
    double max{0};
    unsigned int targetBin{0}; // Location of fund. freq. will be stored here.
    auto data = getFFTData();

    // Finds fft bin with most energy.
    for (unsigned int i = 0; i < getFFTSize(); ++i) {
        if (max < data[i]) {
            max = data[i];
            targetBin = i;
        }
    }

    // Calculates frequency from bin number and accesses amplitude at bin
    // number.
    auto fundamental = std::make_pair<double, double>(
        (double)targetBin * sampleRate / (fftSize * 2),
        (double)(data[targetBin] / fftSize));
    return fundamental;
}

std::vector<std::pair<int, double>>
ForwardFFT::getHarmonics(const unsigned int &numPartials,
                         const std::vector<double> &noteFreq) {
    auto data = getFFTData();
    cleanUpBins(data);
    auto notes = mapBinsToNotes(noteFreq, data);
    return determineHarmonics(numPartials, notes);
}

int ForwardFFT::getWindowingFunction() const { return winMethod; }

void ForwardFFT::cleanUpBins(std::array<float, fftSize * 2> &data) {
    constexpr double threshold{1.0};

    std::vector<int> lobes;
    for (int bin = 0; bin < getFFTSize(); ++bin) {
        // Clean up noise - acts like a gate.
        if (data[bin] < threshold) {
            data[bin] = 0;
        } else {
            // Adds bin as part of a lobe when above threshold.
            lobes.push_back(bin);
        }

        // When bin is zero, we've moved past the lobe and it can be analyzed.
        // Squeezes lobe into a sigle bin, being the center bin of the lobe.
        if (lobes.size() > 0 && data[bin] == 0) {
            float maxLobe{0.0};
            int ctrBin{0};
            for (int i : lobes) {
                if (data[i] > maxLobe) {
                    ctrBin = i;
                    maxLobe = data[i];
                }
            }

            for (int i : lobes) {
                // Adds all amplitudes to center bin.
                if (i != ctrBin) {
                    data[ctrBin] += data[i];
                    data[i] = 0;
                }
            }
            lobes.clear();
        }
    }
}

std::vector<double>
ForwardFFT::mapBinsToNotes(const std::vector<double> &noteFreq,
                           const std::array<float, fftSize * 2> &data) {
    // Determines closest note to all bins in FFT and maps bins to their correct
    // frequencies. The amplitudes of each bin is added onto the notes
    // amplitude.
    std::vector<double> amps(noteFreq.size());
    int i = 0;
    for (int bin = 1; bin < getFFTSize(); ++bin) {
        double freq = (double)bin * sampleRate / (fftSize * 2);
        int note = anyMidi::findNearestNote(freq, noteFreq);

        amps[note] += data[bin];
    }

    return amps;
}

std::vector<std::pair<int, double>>
ForwardFFT::determineHarmonics(const unsigned int &numPartials,
                               std::vector<double> &amps) const {
    // Thanks to
    // https://stackoverflow.com/questions/14902876/indices-of-the-k-largest-elements-in-an-unsorted-length-n-array/38391603#38391603
    // for inspiration for this algorithm.

    // Stores bin index and value for the n loudest partials.
    std::priority_queue<std::pair<double, int>,
                        std::vector<std::pair<double, int>>,
                        std::greater<std::pair<double, int>>>
        queue;

    // Puts the loudest bins into the priority queue, storing the amplitude and
    // the index.
    for (int i = 0; i < amps.size(); ++i) {
        if (queue.size() < numPartials) {
            queue.push(std::pair<double, int>{amps[i], i});
        } else if (queue.top().first < amps[i]) {
            queue.pop();
            queue.push(std::pair<double, int>{amps[i], i});
        }
    }

    // Transforms the priority queue into a variable size vector, since the
    // number of partials may change.
    int k = queue.size();
    std::vector<std::pair<int, double>> harmonics;
    for (int i = 0; i < k; ++i) {
        // Creates pair of {frequenzy, amplitude}.
        harmonics.push_back(
            std::make_pair(queue.top().second, queue.top().first / fftSize));
        queue.pop();
    }

    // Sorts harmonics based on lowest frequency.
    std::sort(harmonics.begin(), harmonics.end(),
              [](std::pair<int, double> a, std::pair<int, double> b) {
                  return a.first < b.first;
              });

    return harmonics;
}

int anyMidi::findNearestNote(const double &target,
                             const std::vector<double> &noteFrequencies) {
    unsigned int begin = 0;
    unsigned int end = noteFrequencies.size();

    // When frequency is below or above highest midi note.
    if (target <= noteFrequencies[begin]) {
        return begin;
    }
    if (target >= noteFrequencies[end - 1]) {
        return end - 1;
    }

    // Variation of binary search.
    unsigned int mid;
    while (end - begin > 1) {
        mid = begin + (end - begin) / 2;

        if (target == noteFrequencies[mid]) {
            return mid;
        }
        if (target < noteFrequencies[mid]) {
            end = mid;
        } else {
            begin = mid;
        }
    }

    if (std::abs(target - noteFrequencies[begin]) <
        std::abs(target - noteFrequencies[end])) {
        return begin;
    } else {
        return end;
    }
}