FROM debian:12.11

RUN apt update && \
    apt install -y \
        curl \
        clang-tidy-15 \
        g++-13 \
        ninja-build \
        libasound2-dev \
        libjack-jackd2-dev \
        ladspa-sdk \
        libcurl4-openssl-dev \
        libfreetype6-dev \
        libx11-dev \
        libxcomposite-dev \
        libxcursor-dev \
        libxext-dev \
        libxinerama-dev \
        libxrandr-dev \
        libxrender-dev \
        libwebkit2gtk-4.0-dev

RUN mkdir /juce
RUN curl -s -L https://github.com/juce-framework/JUCE/archive/refs/tags/7.0.9.tar.gz | \
    tar xzvf - -C /juce