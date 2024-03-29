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

anyMidi::ForwardFFT::ForwardFFT(
    const double sampleRate,
    const juce::dsp::WindowingFunction<float>::WindowingMethod windowingMethod)
    : forwardFFT_{fftOrder}, sampleRate_{sampleRate},
      // When initialising the windowing function, consider using fftSize + 1,
      // ref. https://artandlogic.com/2019/11/making-spectrograms-in-juce/amp/
      window_{fftSize + 1, windowingMethod}, winMethod_{windowingMethod} {
    windowCompensation_ = windowCompensations_.at(windowingMethod);
}

std::array<float, anyMidi::ForwardFFT::fftSize * 2>
anyMidi::ForwardFFT::getFFTData() const {
    return fftData_;
}

int anyMidi::ForwardFFT::getFFTSize() { return fftSize; }

void anyMidi::ForwardFFT::setWindowingFunction(const int &id) {
    auto winMethod =
        static_cast<juce::dsp::WindowingFunction<float>::WindowingMethod>(id);

    this->winMethod_ = winMethod;
    this->windowCompensation_ = windowCompensations_.at(winMethod);
    this->window_.fillWindowingTables(fftSize + 1, winMethod);
}

juce::Array<juce::String>
anyMidi::ForwardFFT::getAvailableWindowingMethods() const {
    juce::Array<juce::String> windowStrings;
    windowStrings.resize(juce::dsp::WindowingFunction<
                         float>::WindowingMethod::numWindowingMethods);
    windowStrings.fill("");

    // Fill array with existing window method enums.
    for (const auto &w : windowCompensations_) {
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

void anyMidi::ForwardFFT::pushNextSampleIntoFifo(float sample) {
    // When fifo contains enough data, flag is set to say next frame should be
    // rendered.
    if (fifoIndex_ == fftSize) {
        if (!nextFFTBlockReady_) {
            // Initializes fftData with zeroes.
            std::fill(fftData_.begin(), fftData_.end(), 0.0F);
            // Copies the data from the fifo into fftData.
            std::copy(fifo_.begin(), fifo_.end(), fftData_.begin());
            // Sets flag.
            nextFFTBlockReady_ = true;

            // Perform windowing and forward FFT.
            window_.multiplyWithWindowingTable(fftData_.data(), fftSize);
            forwardFFT_.performFrequencyOnlyForwardTransform(fftData_.data());

            // Amplitude compensation for window function.
            juce::FloatVectorOperationsBase<float, size_t>::multiply(
                fftData_.data(), windowCompensation_, fftSize);
        }

        fifoIndex_ = 0;
    }

    fifo_.at(fifoIndex_++) = sample;
}

std::pair<double, double> anyMidi::ForwardFFT::calcFundamentalFreq() const {
    double max{0};
    unsigned int targetBin{0}; // Location of fund. freq. will be stored here.
    auto data = getFFTData();

    // Finds fft bin with most energy.
    for (int i = 0; i < getFFTSize(); ++i) {
        if (max < data.at(i)) {
            max = data.at(i);
            targetBin = i;
        }
    }

    // Calculates frequency from bin number and accesses amplitude at bin
    // number.
    auto fundamental = std::make_pair<double, double>(
        static_cast<double>(targetBin * sampleRate_ / (fftSize * 2)),
        static_cast<double>((data.at(targetBin) / fftSize)));
    return fundamental;
}

std::vector<std::pair<int, double>>
anyMidi::ForwardFFT::getHarmonics(const unsigned int &numPartials,
                                  const std::vector<double> &noteFreq) const {
    auto data = getFFTData();
    cleanUpBins(data);
    auto notes = mapBinsToNotes(noteFreq, data);
    return determineHarmonics(numPartials, notes);
}

int anyMidi::ForwardFFT::getWindowingFunction() const { return winMethod_; }

void anyMidi::ForwardFFT::cleanUpBins(std::array<float, fftSize * 2> &data) {
    constexpr double kThreshold{1.0};

    std::vector<int> lobes;
    for (int bin = 0; bin < getFFTSize(); ++bin) {
        // Clean up noise - acts like a gate.
        if (data.at(bin) < kThreshold) {
            data.at(bin) = 0;
        } else {
            // Adds bin as part of a lobe when above threshold.
            lobes.push_back(bin);
        }

        // When bin is zero, we've moved past the lobe and it can be analyzed.
        // Squeezes lobe into a single bin, being the center bin of the lobe.
        if (!lobes.empty() && data.at(bin) == 0) {
            float maxLobe{0.0};
            int ctrBin{0};
            for (const int i : lobes) {
                if (data.at(i) > maxLobe) {
                    ctrBin = i;
                    maxLobe = data.at(i);
                }
            }

            for (const int i : lobes) {
                // Adds all amplitudes to center bin.
                if (i != ctrBin) {
                    data.at(ctrBin) += data.at(i);
                    data.at(i) = 0;
                }
            }
            lobes.clear();
        }
    }
}

std::vector<double> anyMidi::ForwardFFT::mapBinsToNotes(
    const std::vector<double> &noteFreq,
    const std::array<float, fftSize * 2UL> &data) const {
    // Determines closest note to all bins in FFT and maps bins to their correct
    // frequencies. The amplitudes of each bin is added onto the notes
    // amplitude.
    std::vector<double> amps(noteFreq.size());
    for (int bin = 1; bin < getFFTSize(); ++bin) {
        const auto freq =
            static_cast<double>(bin * sampleRate_ / (fftSize * 2));
        auto note = anyMidi::findNearestNote(freq, noteFreq);

        amps[note] += data.at(bin);
    }

    return amps;
}

std::vector<std::pair<int, double>>
anyMidi::ForwardFFT::determineHarmonics(const unsigned int &numPartials,
                                        std::vector<double> &amps) {
    // Thanks to
    // https://stackoverflow.com/questions/14902876/indices-of-the-k-largest-elements-in-an-unsorted-length-n-array/38391603#38391603
    // for inspiration for this algorithm.

    // Stores bin index and value for the n loudest partials.
    std::priority_queue<std::pair<double, int>,
                        std::vector<std::pair<double, int>>, std::greater<>>
        queue;

    // Puts the loudest bins into the priority queue, storing the amplitude and
    // the index.
    for (int i = 0; i < amps.size(); ++i) {
        if (queue.size() < numPartials) {
            queue.emplace(amps[i], i);
        } else if (queue.top().first < amps[i]) {
            queue.pop();
            queue.emplace(amps[i], i);
        }
    }

    // Transforms the priority queue into a variable size vector, since the
    // number of partials may change.
    auto k = queue.size();
    std::vector<std::pair<int, double>> harmonics;
    for (int i = 0; i < k; ++i) {
        // Creates pair of {frequenzy, amplitude}.
        harmonics.emplace_back(queue.top().second, queue.top().first / fftSize);
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
    int begin{0};
    auto end = static_cast<int>(noteFrequencies.size());

    // When frequency is below or above highest midi note.
    if (target <= noteFrequencies[begin]) {
        return begin;
    }
    if (target >= noteFrequencies[end - 1]) {
        return end - 1;
    }

    // Variation of binary search.
    int mid{0};
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
    }

    return end;
}