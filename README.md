# anyMidi

Project for my bachelor thesis in Music technology.

## Requirements

To be able to build the application users need the JUCE framework installed, which can be downloaded for free from here: https://juce.com/get-juce/download

There is need for a separate driver for routing MIDI from anyMidi to your DAW or other app. **LoopBe1** has been tested during development of anyMidi and can be downloaded here: https://www.nerds.de/en/download.html

For Mac users there is a way to set up loopback drivers for MIDI (https://www.youtube.com/watch?v=MK4hrjfJEX4), although this has not been tested with the application.

## Build

1. Open the anyMidiStandalone.jucer file. Make sure the global paths are correct by going to File>Global Paths and see that "Path to JUCE" and "JUCE Modules" are pointing to your JUCE install directory and the modules directory, respectively.

2. Close the window and select your exporter (e.g. Visual Studio or Xcode). Click the icon to the right to launch the selected IDE.

3. From the IDE, build the project as you would normally do.

## Configuration

LoopBe1 (or similar) and your DAW needs to be running when anyMidi is in use. From the anyMidi UI audio input can be selected and the MIDI output needs to be LoopBe1. In your DAW, select LoopBe1 as a MIDI input. Now you should be able to produce MIDI in you DAW by playing on your connected instrument!
