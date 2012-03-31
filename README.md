# Ground Track Generator

A utility to write satellite ground tracks as GIS-compatible [shapefiles](https://en.wikipedia.org/wiki/Shapefile). Orbits are modelled with Dan Warner's [C++ SGP4 library](http://www.danrw.com/sgp4-satellite.php), based on ["Revisiting Spacetrack Report #3"](http://www.celestrak.com/publications/AIAA/2006-6753/), by David Vallado, Paul Crawford, Richard Hujsak, and [T.S. Kelso](http://www.celestrak.com/webmaster.asp). [Two-line element](http://celestrak.com/NORAD/elements/) (TLE) sets of orbit parameters are read as input. [Shapelib 1.3.0b3](http://shapelib.maptools.org/) is used to generate the shapefiles. 

![stereographic landsat birthday trace](https://github.com/anoved/Ground-Track-Generator/raw/master/test/images/ls83.png)

## Features

Ground Track Generator can represent each step of the ground track as points or line segments. The step size is determined by a user-specified time interval. The extent of the ground track is controlled by a start time (absolute or relative to the TLE epoch) and either an end time or a step count. A variety of attributes can be output for each step, including elevation and azimuth as viewed from an optionally-specified ground observer. Here, intervals of an orbit with positive elevation as viewed from the northeast US are symbolized with increasingly large circles:

![trace with observer attributes](https://github.com/anoved/Ground-Track-Generator/raw/master/test/images/elevation-trace.png)

## Building

To compile Ground Track Generator you need two small libraries. See `Libraries/README.md` for setup details (it's a matter of downloading two zip files, dragging their contents to `Libraries`, and installing the provided Makefiles).

Once that's done, the following should be sufficient to build Ground Track Generator:

	make gtg

To run the test cases described in `test/cases/README.md`, try:

	make test

Note that errors in test cases 12, 23, 26, 27, 30, 31, and 33 are an intentional part of the test suite.

## Usage

	Ground Track Generator 0.1
	usage: gtg [OPTIONS] [TLE [TLE ...]]
	
	Ground Track Generator outputs GIS-compatible shapefiles containing point or
	line segment representations of the ground track of specified satellite orbits.
	The extent and resolution of the ground track is controlled by the TRACE OPTIONS
	described below. Orbit information is read from two-line element sets and the
	orbit is modelled with the SGP4/SDP4 simplified perturbation model.
	
	OPTIONS:
	
	  INPUT OPTIONS:
	
		--tle/-t TEXT
			Load two-line element sets directly from TEXT.
	
		--input/-i PATH
			Load two-line element sets from file PATH.
	
		TLE [TLE ...]
			Any command line arguments not interpreted as the options or arguments
			described here are treated as the PATH to two-line element set files.
	
		If no two-line element sets (TLEs) are loaded from the command line, gtg
		will attempt to read two-line element sets from standard input. If multiple
		TLEs are loaded, a separate shapefile will be output for each TLE.
	
	  OUTPUT OPTIONS:
	
		--output/-o PATH | DIRECTORY
			If a single two-line element set is loaded, specify the base PATH of the
			output (defaults to the TLE identifier described below).
			If multiple two-line element sets are loaded, specify the directory in
			which to write output files (defaults to current working directory).
	
		--prefix/-p PREFIX
			If specified, PREFIX is prepended to the base name identifier, unless
			there is only one two-line element set and an --output PATH is given.
	
		--suffix/-x SUFFIX
			If specified, SUFFIX is appended to the base name identifier, unless
			there is only one two-line element set and an --output PATH is given.
	
		--noprj
			Suppress output of .prj "projection" file, which explicitly specifies
			the geodetic reference system of the generated shapefile (WGS-72).
	
		The default base name for output files is the [NORAD] satellite number
		encoded in the second field of the first line of the two-line element set.
	
	  GEOMETRY OPTIONS:
	
		--features/-f point | line
			Specify whether to output points (the default) or line segment features
			for each step. Attributes refer to the starting point of line features.
	
		--split/-d
			If generating line --features, split any lines that cross the 180th
			meridian into east and west hemisphere segments. Disabled by default.
		  IMPORTANT: --split is intended as a cosmetic convenience only. The split
			point latitude is not determined with the same precision as the trace.
	
	  ATTRIBUTE OPTIONS:
	
		--attributes/-a all | standard | ATTRIBUTE [ATTRIBUTE ...]
			By default, no attributes are output.
				all       - Output all attributes. Some require an --observer.
				standard  - All attributes except those which require an observer.
			Alternatively, list one or more of the following ATTRIBUTE names:
				time      - Step timestamp in YYYY-MM-DD HH:MM:SS.SSSSSS UTC
				unixtime  - Step timestamp in seconds since 0:0:0 UTC 1 Jan 1970.
				mfe       - Relative timestamp in minutes from epoch.
				altitude  - Altitude of satellite in km.
				velocity  - Magnitude of satellite velocity in km/s.
				latitude  - Geodetic latitude of satellite position.
				longitude - Geodetic longitude of satellite position.
				xposition - Earth Centered Inertial (ECI) x position in km.
				yposition - Satellite ECI y position in km.
				zposition - Satellite ECI z position in km.
				xvelocity - Satellite ECI x velocity in km/s.
				yvelocity - Satellite ECI y velocity in km/s.
				zvelocity - Satellite ECI z velocity in km/s.
				range     - Range to satellite from observer in km.
				rate      - Rate of satellite range from observer in km/s.
				elevation - Elevation to satellite from observer.
				azimuth   - Azimuth to satellite from observer.
			Attributes are output in this order regardless of order specified.
	
		--observer/-g LATITUDE LONGITUDE [ALTITUDE]
			Specify the surface location of an observer (optional altitude in km).
			Some --attributes require an observer to be defined. None by default.
	
	  TRACE OPTIONS:
	
		--start/-s now | epoch | TIME | UNIXTIME
			Timestamp for first step of output. Subsequent steps are output at
			uniform time --intervals until --end time or --steps count is reached.
				now[OFFSET]   - Current time, with optional offset.
				epoch[OFFSET] - TLE reference time, with optional offset. Default.
				TIME          - Time in "YYYY-MM-DD HH:MM:SS.SSSSSS UTC" format.
				UNIXTIME      - Time in seconds since 0:0:0 UTC 1 Jan 1970.
			OFFSET format is a +/- number followed by s, m, h, or d, indicating the
			offset unit (seconds, minutes, hours, or days, respectively).
	
		--end/e now | epoch | TIME | UNIXTIME
			If specified, trace is output from --start to no later than --end. If
			not specified, trace is output for the specified number of --steps.
			Same argument format as --start. The --end time must be later than the
			--start time. The time interval between --start and --end must be
			greater than the step --interval.
	
		--forceend
			Causes a final feature to be output exactly at --end time, regardless of
			interval. Has no effect if --end is not specified.
	
		--steps/-n STEPS
			Number of steps to output. Defaults to 1. Ignored if --end is given.
	
		--interval/-l DURATION
			Step interval. Duration format is a number followed by s, m, h, or d,
			indicating the unit (seconds, minutes, hours, or days, respectively).
	
	  MISCELLANEOUS OPTIONS:
	
		--verbose
			Print status messages (including coordinates and attribute values).
	
		--help/-?
			Display this usage message.
	
		--version/-v
			Display the program version.
	
	CREDITS:
	
		C++ SGP4 Satellite Library:
		<http://www.danrw.com/sgp4-satellite.php>
	
		Shapefile C Library:
		<http://shapelib.maptools.org/>
	
		Revisiting Spacetrack Report #3 (background reference and test cases):
		<http://www.celestrak.com/publications/AIAA/2006-6753/>
	

