FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    gcc-arm-none-eabi \
    libnewlib-arm-none-eabi \
    libusb-1.0-0-dev \
    git \
    python3 \
    python3-pip \
    python3-venv \
    wget \
    curl \
    sudo \
    minicom \
    usbutils \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

RUN useradd -m -s /bin/bash developer \
    && usermod -aG dialout,plugdev,root developer \
    && echo 'developer ALL=(ALL) NOPASSWD: ALL' >> /etc/sudoers.d/developer

USER developer

RUN mkdir -p $HOME/.pico-sdk \
    && git clone -b master https://github.com/raspberrypi/pico-sdk.git $HOME/.pico-sdk/pico-sdk \
    && cd $HOME/.pico-sdk/pico-sdk \
    && git submodule update --init

# C/C++ SDK Configuration
RUN echo 'export PICO_SDK_PATH=$HOME/.pico-sdk/pico-sdk' >> ~/.bashrc \
    && echo 'export PATH=$HOME/.pico-sdk/picotool/*/picotool:$PATH' >> ~/.bashrc \
    && echo 'export PATH=$HOME/.pico-sdk/cmake/*/bin:$PATH' >> ~/.bashrc \
    && echo 'export PATH=$HOME/.pico-sdk/ninja/*:$PATH' >> ~/.bashrc

# Micropython
RUN python3 -m pip install --user pyserial mpremote adafruit-ampy rshell \
    && echo 'export PATH=$HOME/.local/bin:$PATH' >> ~/.bashrc