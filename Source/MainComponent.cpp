#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() :
    fft{ 48000 },
    midiProc{ 48000, juce::Time::getMillisecondCounterHiRes() * 0.001 },
    spectrogramImage{ juce::Image::RGB, 512, 512, true },
    audioSetupComp{
        deviceManager,
        0,      // min input ch
        256,    // max input ch
        0,      // min output ch
        256,    // max output ch
        false,  // can select midi inputs?
        true,   // can select midi output device?
        false,  // treat channels as stereo pairs
        false } // hide advanced options?
{
    // Some platforms require permissions to open input channels so request that here.
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        juce::File deviceSettingsFile = juce::File::getCurrentWorkingDirectory().getChildFile("audio_device_settings.xml");
        
        if (deviceSettingsFile.existsAsFile())
        {
            // Loads settings from file if it exists.
            const auto storedSettings = juce::parseXML(deviceSettingsFile);
            setAudioChannels(numInputChannels, numOutputChannels, storedSettings.get());
        }
        else
        {
            setAudioChannels(numInputChannels, numOutputChannels);
        }
    }
    
    // Audio device manager.
    addAndMakeVisible(audioSetupComp);

    // Gain slider.
    addAndMakeVisible(gainSlider);
    gainSlider.setRange(0, 1, 0.01);
    gainSlider.setValue(0.0);

    // Output box, used for debugging.
    addAndMakeVisible(outputBox);
    outputBox.setMultiLine(true);
    outputBox.setReturnKeyStartsNewLine(true);
    outputBox.setReadOnly(true);
    outputBox.setScrollbarsShown(true);
    outputBox.setCaretVisible(false);
    outputBox.setPopupMenuEnabled(true);
    outputBox.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x32ffffff));
    outputBox.setColour(juce::TextEditor::outlineColourId, juce::Colour(0x1c000000));
    outputBox.setColour(juce::TextEditor::shadowColourId, juce::Colour(0x16000000));

    addAndMakeVisible(clearOutput);
    clearOutput.setButtonText("Clear output");
    clearOutput.onClick = [this]
    {
        outputBox.clear();
    };

    // Timer used for messages.
    startTimer(500);

    setSize(1000, 600);

    // Generates a list of frequencies corresponding to the 128 Midi notes
    // based on the global tuning.
    // Thanks to http://subsynth.sourceforge.net/midinote2freq.html for this snippet.
    for (int i = 0; i < 140; ++i)
    {
        noteFrequencies.push_back((tuning / 32.0) * std::pow(2, (i - 9.0) / 12.0));
    }
}

MainComponent::~MainComponent()
{   
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    processingBuffer.setSize(numInputChannels, samplesPerBlockExpected, false, true);
    midiProc.setMidiOutput(deviceManager.getDefaultMidiOutput());

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlockExpected;
    spec.numChannels = numOutputChannels;

    noiseGate.prepare(spec);
    noiseGate.setThreshold(-1.0); //50
    noiseGate.setAttack(5.0);
    noiseGate.setRelease(17.0);
    noiseGate.setRatio(1.0);

    // Initializing highpass filter.
    hiPassFilter.setCoefficients(juce::IIRCoefficients::makeHighPass(sampleRate, 75.0, 1.0)); // E5 on guitar = ~82 Hz
    hiPassFilter.reset();
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (bufferToFill.buffer->getNumChannels() > 0)
    {
        int numSamples = bufferToFill.buffer->getNumSamples(); // To make sure copied buffer size is the same.

        // Copies actual buffer into a buffer for processing.
        for (int i = 0; i < processingBuffer.getNumChannels(); ++i)
        {
            processingBuffer.copyFrom(i, 0, bufferToFill.buffer->getReadPointer(i), numSamples);
        }

        juce::dsp::AudioBlock<float> block{ processingBuffer };
        juce::dsp::ProcessContextReplacing<float> context{ block };
        //noiseGate.process(context);

        auto* channelData = processingBuffer.getReadPointer(0, bufferToFill.startSample);
        auto* outBuffer = processingBuffer.getWritePointer(0, bufferToFill.startSample);
        
        // Puts samples into non-const buffer.
        for (unsigned int i = 0; i < numSamples; ++i)
        {
            outBuffer[i] = channelData[i];
        }

        // Applies filter.
        hiPassFilter.processSamples(outBuffer, numSamples);

        // Puts samples into FFT fifo after processing.
        for (unsigned int i = 0; i < numSamples; ++i)
        {
            fft.pushNextSampleIntoFifo(outBuffer[i]);
        }

        // Just for audio output. Can be removed --
        for (int i = 0; i < processingBuffer.getNumChannels(); ++i)
        {
            bufferToFill.buffer->copyFrom(i, 0, processingBuffer, i, 0, numSamples);
        }

        auto* read = bufferToFill.buffer->getReadPointer(0, bufferToFill.startSample);
        auto* write_l = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
        auto* write_r = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);
        for (unsigned int i = 0; i < numSamples; ++i)
        {
            write_l[i] = read[i] * gainSlider.getValue();
            write_r[i] = read[i] * gainSlider.getValue();
        }
        // -- can be removed
    }

    if (fft.nextFFTBlockReady)
    {       
        calcNote();
        fft.nextFFTBlockReady = false;
    }

    midiProc.pushBufferToOutput();
}

void MainComponent::releaseResources()
{
    auto audioDeviceSettings = audioSetupComp.deviceManager.createStateXml();

    if (audioDeviceSettings != nullptr)
    {
        // Writes user settings to XML file for storage.
        juce::File settingsFileName = juce::File::getCurrentWorkingDirectory().getChildFile("audio_device_settings.xml");
        settingsFileName.replaceWithText(audioDeviceSettings->toString());
    }

    midiProc.turnOffAllMessages();
}

void MainComponent::timerCallback()
{
    /*if (messagesPending.test_and_set())
    {
        drawNextLineOfSpectrogram();

        for (auto m : midiBuffer)
        {
            log(m.getMessage());
        }

        messagesPending.clear();
    }*/
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // Draws spectrogram underneath GUI. It works for now.
    g.drawImage(spectrogramImage, getLocalBounds().toFloat());
}

void MainComponent::resized()
{
    auto rect = getLocalBounds();
    
    auto halfWidth = getWidth() / 2;
    auto halfHeight = getHeight() / 2;

    audioSetupComp.setBounds(rect.withWidth(halfWidth));

    clearOutput.setBounds(rect.getCentreX(), 15, halfWidth / 2 - 10, 20);

    gainSlider.setBounds(rect.getCentreX(), 45, halfWidth / 2 - 10, 20);

    outputBox.setBounds(halfWidth, 85, halfWidth - 10, halfHeight + 40);
}

void MainComponent::drawNextLineOfSpectrogram()
{
    auto rightHandEdge = spectrogramImage.getWidth() - 1;
    auto imageHeight = spectrogramImage.getHeight();
    spectrogramImage.moveImageSection(0, 0, 1, 0, rightHandEdge, imageHeight);

    auto fftData = fft.getFFTData();
    int fftSize = fft.getFFTSize();
        
    auto fftRange = juce::FloatVectorOperations::findMinAndMax(fftData.data(), fftSize / 2);

    for (auto y = 1; y < imageHeight; ++y)
    {
        float skewedProportionY = 1.0f - std::exp(std::log((float)y / (float)imageHeight) * 0.2f);
        size_t fftDataIndex = (size_t)juce::jlimit(0, fftSize / 2, (int)(skewedProportionY * fftSize / 2 ));
        float level = juce::jmap(fftData[fftDataIndex], 0.0f, juce::jmax(fftRange.getEnd(), 1e-5f), 0.0f, 1.0f);

        spectrogramImage.setPixelAt(rightHandEdge, y, juce::Colour::fromHSV(level, 1.0f, level, 1.0f));
    }
}

void MainComponent::calcNote()
{
    // Getting fundamental frequency from FFT and calculating midi note number with velocity.
    /*auto fund = fft.calcFundamentalFreq();
    double amp = fund.second;
    unsigned int note = findNearestNote(fund.first);
    int velocity = (int)std::round(amp * 127.0);*/

    auto noteInfo = analyzeHarmonics(); // Gets {note, amplitude}
    int note = noteInfo.first;
    double amp = noteInfo.second;
    int velocity = (int)std::round(amp * 127);

    std::vector<bool> noteValues;
    if (midiProc.determineNoteValue(note, amp, noteValues))
    {
        for (bool value : noteValues)
        {
            if (value == true)
            {
                midiProc.createMidiMsg(note, velocity, value);
            }
            else
            {
                // Problems with turning correct midi note off led to just turning
                // everything off. This works for monophonic playing.
                midiProc.turnOffAllMessages();
            }
        }
    }
}

std::pair<int, double> MainComponent::analyzeHarmonics()
{
    constexpr int numHarm{ 5 };
    auto harmonics = fft.getHarmonics(numHarm, noteFrequencies);

    int fundamental{ 0 };
    double maxAmp{ 0.0 };
    std::map<int, double> scores;
    double totalAmp{ 0.0 };
    for (int i = 0; i < numHarm; ++i)
    {
        int note = harmonics[i].first; //findNearestNote(harmonics[i].first);
        
        // Extracts the note letter information, without regards to the octave.
        int noteLetter = note % 12;
        
        // Calculates a weighted scoring of each harmonic.
        // The octave-internal note value gets a score, weighted by the number harmonic.
        // Higher harmonics are more accurate, and thus will give higher score towards note.
        
        // double score = (double)(i + 1) / numHarm;
        double score = 1.0;

        scores[noteLetter] += score * (double)(i + 1)/harmonics.size();

        if (i >= 2)
        {
            int tonicOfThird = (12 + (noteLetter - 4)) % 12;
            int tonicOfFifth = (12 + (noteLetter - 7)) % 12;

            scores[tonicOfThird] += score * 0.75;
            scores[tonicOfFifth] += score;
        }

        // Fundamental frequency determines which octave the note will be in.
        if (i == 0/*harmonics[i].second > maxAmp*/)
        {
            fundamental = note;
        }

        totalAmp += harmonics[i].second;
    }

    // Gets the octave number of note from integer divison of fundamental note.
    int octave = fundamental / 12;

    // Finds note with highest score.
    int correctNote{ 0 };
    double maxScore{ 0.0 };
    for (std::pair<int, double> s : scores)
    {
        if (s.second > maxScore)
        {
            correctNote = s.first;
            maxScore = s.second;
        }
    }

    // The note determined by the analyzation of harmonics is composed of the
    // octave of the fundamental note, plus the note value of the harmonic highest scoring.
    int analyzedNoteValue = octave * 12 + correctNote;

    std::pair<int, double> analyzedNote = std::make_pair(analyzedNoteValue, totalAmp);
    return analyzedNote;
    // * Finn sterkeste overtone.
    // * Heltallsdivider på 12, finn nummeret på oktavet. Grunnfrekvensen bestemmer oktav.
    // * Iterer gjennom overtoner og finn modulusen som representerer noteverdien innad i et oktav.
    //   Nye noter legges inn i vektor og får en vektet poengscore, hvor lysere overtoner teller mer.
    // * Til slutt vil en note ha mest score og er dermed den korrekte tonen. Noteverdien legges sammen
    //   med oktavverdien og produserer den korrekte notebokstaven i korrekt oktav.
}

void MainComponent::log(const juce::MidiMessage& midiMessage)
{
    outputBox.moveCaretToEnd();
    outputBox.insertTextAtCaret(midiMessage.getDescription() + juce::newLine);
}

template<typename T>
void MainComponent::log(T msg)
{
    outputBox.moveCaretToEnd();
    outputBox.insertTextAtCaret(std::to_string(msg) + juce::newLine);
}

int MainComponent::findNearestNote(double target)
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

    if (target - noteFrequencies[begin] < target - noteFrequencies[end])
    {
        return begin;
    }
    else
    {
        return end;
    }
}