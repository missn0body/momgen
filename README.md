# momgen

A stripped-down, simplistic Makefile generator.  
Created in 2024 by anson.

You see, I am lazy, and I try to find the easiest and least time consuming
ways to do everything, including creating Makefiles. This program was designed
for me first and foremost, and while I think this could benefit someone out
there, there are still some idiosyncrasies present in the output of this program.

Within the version folders is a `doc` folder that holds sample generated Makefiles
to have an idea of what this program generates. This folder also holds the thesis
statement for that version.

Usage and options are explained in the first section of `src/momgen.cpp`, and can
be read by invoking `momgen --help` at the command line. This project is licensed
under the MIT License, See LICENSE for related details. Issues, bugs, and other
things can be discussed at my E-Mail, <thesearethethingswesaw@gmail.com>

### v.1.0.0 (Initial release)

(Early March 2024)  
A stripped-down, simplistic Makefile generator created by anson.

### v.2.0.0 (C++ Upgrade)

(Late March 2024)  
A stripped-down, simplistic Makefile generator created by anson.
* Moved from C to C++
* Expanded project structure from one contained file to multiple files in organized folders
* Changed from ad-hoc argument parser to [Docopt](http://docopt.org)
