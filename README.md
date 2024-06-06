# momgen

A stripped-down, simplistic Makefile generator.  
Created in 2024 by anson.

You see, I am lazy, and I try to find the easiest and least time consuming
ways to do everything, including creating Makefiles. This program was designed
for me first and foremost, and while I think this could benefit someone out
there, there are still some idiosyncrasies present in the output of this program.

Within the version folders is a `doc` folder that holds the `man` page for that
version of the program. This folder also holds the thesis statement for that version.

Usage and options are explained in the first section of `src/momgen.cpp`, and can
be read by invoking `momgen --help` at the command line. This project is licensed
under the MIT License, see LICENSE for related details. Issues, bugs, and other
things can be discussed at my E-Mail, <thesearethethingswesaw@gmail.com>

### v.1.0.0 (Initial release)

(Early March 2024)  
A stripped-down, simplistic Makefile generator created by anson.

### v.2.0.0 (C++ Upgrade)

(Early April 2024)  
A stripped-down, simplistic Makefile generator created by anson.
* Moved from C to C++
* Expanded project structure from one contained file to multiple files in organized folders
* Changed from ad-hoc argument parser to [Docopt](http://docopt.org)

### v.2.0.2

(Mid April 2024)  
A stripped-down, simplistic Makefile generator created by anson.
* Fixed missing BIN variable
* Fixed a bug where a linking build rule is not correctly a label
* Fixed a bug where CCFLAGS is used instead of CFLAGS
* Switched CCFLAGS for CXXFLAGS
* Fixed a bug where BDIR/BIN was being used instead of BIN when making single-file distributions
* Fixed overwriting of a previous Makefile when previewing the output of `momgen`
* Added .PHONY rule

### v.2.0.3

(June 2024)  
A stripped-down, simplistic Makefile generator created by anson.
* Fixed a bug where DDIR variable wasn't generated with single file project Makefiles set up with distribution rules
* Fixed spacing issues with the `dist` rule
