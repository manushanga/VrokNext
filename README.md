README
======

Building
========

macOS (CMake)
-------------

The CMake build produces `libvrok.dylib` and the `simple_main` player. It uses
FFmpeg (`libavformat`, `libavcodec`, `libavutil`), libao's native macOS backend,
and POSIX threads. Qt is not required. Install Xcode Command Line Tools and
provide CMake, pkg-config, FFmpeg development headers/libraries and libao.

With Nix installed, prepare the dependencies without changing your profile:

```sh
ffmpeg_dev=$(nix build --no-link --print-out-paths nixpkgs#ffmpeg.dev)
ao_dev=$(nix build --no-link --print-out-paths nixpkgs#libao.dev)
export PKG_CONFIG_PATH="$ffmpeg_dev/lib/pkgconfig:$ao_dev/lib/pkgconfig"
nix shell nixpkgs#cmake nixpkgs#pkg-config -c cmake -S . -B build/macos \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_COMPILER=/usr/bin/clang \
    -DCMAKE_CXX_COMPILER=/usr/bin/clang++
nix shell nixpkgs#cmake -c cmake --build build/macos --parallel
```

With dependencies already visible to pkg-config, the equivalent build is:

```sh
cmake -S . -B build/macos -DCMAKE_BUILD_TYPE=Debug
cmake --build build/macos --parallel
./build/macos/simple_main /absolute/path/to/audio.wav
```

The sample player repeats the file; stop it with Ctrl-C. Audio playback needs
access to the macOS audio service and may fail inside a restricted sandbox.
Build dependencies must match the target architecture. Builds using Nix link
to libraries in the Nix store and are not standalone redistributable bundles.

Validated on Apple Silicon with Apple Clang 21, CMake 4.4.3, FFmpeg 9.0.2,
and libao 1.2.2. Existing compiler warnings remain in the decoder and equalizer.

Windows
-------
* Build libAO for Windows, FFmpeg, Qt, and link it with Vrok. It is easier if you use qmake

Linux
-----
* Build or install libAO, libasound, FFmpeg, Qt
* Build using qmake

Linux - with python support
---------------------------
* Build or install libAO, libasound, FFmpeg, Boost Python, Python
* Build using make

Android
-------
* Make sure you have built FFmpeg for Android and fix the paths in Android.mk 
* Build using ndk-build 
* Note that the java application that is bundled has not been commited to this repo yet, whatever you build will have to have another interface or main application

Running
=======

Windows & Linux
---------------

* VrokNext has a command line interface as of now, it supports cd and ls. If you want to change its start up directory input it as an argument to the application.
* To play a file enter its number(index).
* To change properties of plugins use below commands,
    1. setc - sets the current component that you want to make changes to (e.g. setc FIR filter:0)
    2. setp - sets properties of the current component (e.g. setp lp_freq 100, sets lp_freq to 100)

### Supported Commands

`<index>` play the file with the given index  
`open <filename>` play given file  
`openi i` play file with the given index  
`p` pause  
`r` resume  
`ls` similar to Unix ls, lists current directory's contents  
`cd <dir>|<dir index>` similar to Unix cd, changes directory through text and index (shown in ls output)  
`setc <component name>` sets current component  
`setp <property name> <property value>` sets current component's properties

### Currently implemented components and their properties

1. SSEQ (Shibatch Super EQ by Naoki Shibata)  
    18 band equalizer that has following properties  
    preamp = [0.0..inf), default: 1.0  
    band_0 through band_17 = [0.0..inf), default: 1.0

2. FIR filter 
    Despite the name, currently implements a bass enhancer (from Calf plugins for JACK)  
    dry_vol = [0.0..inf), default: 1.0, dry signal mix level  
    wet_vol = [0.0..inf), default: 1.0, wet signal mix level  
    blend = [-10.0..10.0), default: 9.0  
    drive = [0.0..10.0), default: 1.0  
    lp_freq = [0.0..20000.0], default: 150.0, low pass frequency  
    hp_freq = [0.0..20000.0], default: 50.0, high pass frequency  

Linux - with python support
---------------------------

* Execute `python3 testbp.py dir0 [dir1 [dir2 [dir3 [ ... [dirN]]]]]` , dirX are directories that the script will search for media


