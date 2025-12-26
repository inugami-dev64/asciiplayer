# ASCII video player

This repository contains source-code for ascii video player.

## Build on Linux

Step 1: Install prerequisites  
Fedora:
```bash
$ sudo dnf install ffmpeg-libs
```

Debian:
```bash
$ sudo apt install -y libavcodec-dev libavformat-dev libavdevice-dev libavfilter-dev portaudio19-dev libswscale-dev libswresample-dev
```

Step 2: Build the project:  
```bash
$ mkdir build
```
```bash
$ cd build
```
```bash
$ cmake -DCMAKE_BUILD_TYPE=Release ..
```