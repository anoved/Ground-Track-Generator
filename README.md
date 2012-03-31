# Ground Track Generator

A utility to write satellite ground tracks as GIS-compatible [shapefiles](https://en.wikipedia.org/wiki/Shapefile). Orbits are modelled with Dan Warner's [C++ SGP4 library](http://www.danrw.com/sgp4-satellite.php), based on ["Revisiting Spacetrack Report #3"](http://www.celestrak.com/publications/AIAA/2006-6753/), by David Vallado, Paul Crawford, Richard Hujsak, and [T.S. Kelso](http://www.celestrak.com/webmaster.asp). [Two-line element](http://celestrak.com/NORAD/elements/) (TLE) sets of orbit parameters are read as input. [Shapelib 1.3.0b3](http://shapelib.maptools.org/) is used to generate the shapefiles. 

## Features

Ground Track Generator can represent each step of the ground track as points or line segments. The step size is determined by a user-specified time interval. The extent of the ground track is controlled by a start time (absolute or relative to the TLE epoch) and either an end time or a step count. A variety of attributes can be output for each step, including elevation and azimuth as viewed from an optionally-specified ground observer.

## Instructions and Examples

Please see [`Examples/README.md`](https://github.com/anoved/Ground-Track-Generator/blob/master/Examples/README.md) for a guide to getting started with `gtg`, including screenshots, comments on command line options, and example files. This is a good place to start!

## Download

An executable copy of `gtg` for Intel Macs is available here. It is a command line program.

- [gtg-mac-intel-1.0.zip](https://github.com/anoved/Ground-Track-Generator/raw/master/Builds/gtg-mac-intel-1.0.zip)

For the time being, users of other platforms will need to compile `gtg` themselves. Please see the following section.

## Compiling gtg

To compile Ground Track Generator you need two small libraries. See [`Libraries/README.md`](https://github.com/anoved/Ground-Track-Generator/blob/master/Libraries/README.md) for setup details (it's a matter of downloading two zip files, dragging their contents to `Libraries`, and installing the provided Makefiles).

Once that's done, the following should be sufficient to build Ground Track Generator:

	make gtg

To run the test cases described in [`test/cases/README.md`](https://github.com/anoved/Ground-Track-Generator/blob/master/test/cases/README.md), try:

	make test

Note that errors in test cases 12, 23, 26, 27, 30, 31, and 33 are an intentional part of the test suite.

---

![geoeye trace](https://github.com/anoved/Ground-Track-Generator/blob/master/Examples/Images/4.png?raw=true)

![stereographic landsat birthday trace](https://github.com/anoved/Ground-Track-Generator/raw/master/test/images/ls83.png)

![elevation attribute test](https://github.com/anoved/Ground-Track-Generator/blob/master/test/images/elevation-trace.png?raw=true)
