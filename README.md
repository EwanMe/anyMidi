<div align="center">
  <img src="./resources/anyMidiLogo.png" alt="anyMidi" height="60" />
  <h1>anyMidi</h1>
</div>

This project aims to convert audio into MIDI notes with the help of FFT and harmonics analysis. Work on this project started as a part of my bachelor thesis in the field of music technology at the [Norwegian University of Science and Technology (NTNU)](https://www.ntnu.edu/), Trondheim, Norway. Basic functionality was achieved for the presentation of my thesis in June 2021. After that, work has continued on the project at a slower pace, seeking to improve upon the achievements of my thesis.

## :memo: Documentation

Doxygen documentation is built to the branch [doxygen_pages](https://github.com/EwanMe/anyMidi/tree/doxygen_pages), and can be viewed at [ewanme.github.io/anymidi](https://ewanme.github.io/anyMidi).

## :computer: How to get the project running

### :books: Requirements

To be able to build the application users need the JUCE framework installed, which can be downloaded for free from here: https://juce.com/get-juce/download

There is need for a separate driver for routing MIDI from anyMidi to your DAW or other app. **LoopBe1** has been tested during development of anyMidi and can be downloaded here: https://www.nerds.de/en/download.html

For Mac users there is a way to set up loopback drivers for MIDI ([YouTube video here](https://www.youtube.com/watch?v=MK4hrjfJEX4)), although this has not been tested with the application.

There is also need for the [ASIO SDK](https://www.steinberg.net/developers/) to reduce latency considerably. Make sure the paths to the SDK are correct, they can be updated in the .jucer file. Right now these header search paths are used: `D:\dev\asiosdk_2.3.3_2019-06-14\common` and `C:\dev\asiosdk_2.3.3_2019-06-14\common`.

### :hammer: Build

1. Open the anyMidiStandalone.jucer file. Make sure the global paths are correct by going to File>Global Paths and see that "Path to JUCE" and "JUCE Modules" are pointing to your JUCE install directory and the modules directory, respectively.

2. Close the window and select your exporter (e.g. Visual Studio or Xcode). Click the icon to the right to launch the selected IDE.

3. From the IDE, build the project as you would normally do.

### :wrench: Configuration

LoopBe1 (or similar) and your DAW needs to be running when anyMidi is in use. From the anyMidi UI audio input can be selected and the MIDI output needs to be LoopBe1. In your DAW, select LoopBe1 as a MIDI input. Now you should be able to produce MIDI in you DAW by playing on your connected instrument!
