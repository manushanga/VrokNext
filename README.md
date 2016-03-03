README
======

Building
========

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
    Despite the name, currently implements bass enhancer (from Calf plugins for JACK)
    dry_vol = [0.0..inf), default: 1.0, dry signal mix level  
    wet_vol = [0.0..inf), default: 1.0, wet signal mix level  
    blend = [-10.0..10.0), default: 9.0  
    drive = [0.0..10.0), default: 1.0  
    lp_freq = [0.0..20000.0], default: 150.0, low pass frequency  
    lp_freq = [0.0..20000.0], default: 50.0, high pass frequency  

Linux - with python support
---------------------------

* Execute python3 dir0 [dir1 [dir2 [dir3 [ ... [dirN]]]]] , dirX are directories that the script will search for media


