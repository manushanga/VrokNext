VrokNext README
===============

Building
========

Windows
-------
* Build libAO for Windows, FFmpeg, and link it with Vrok. It is easier if you use qmake

Linux
-----
* Build or install libAO and FFmpeg to your system
* Build using qmake

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
* To play a file just insert its number(index) and hit enter
* To change properties of plugins use below commands,
    1. setc - sets the current component that you want to make changes to (e.g. setc FIR filter:0)
    2. setp - sets properties of the current component (e.g. setp lp_freq 100, sets lp_freq to 100)
