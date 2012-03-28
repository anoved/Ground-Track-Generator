# Ground Track Generator

![stereographic landsat birthday trace](https://github.com/anoved/Ground-Track-Generator/raw/master/test/images/ls83.png)

A utility to write satellite ground tracks as GIS-compatible [shapefiles](https://en.wikipedia.org/wiki/Shapefile). Orbits are modelled with Dan Warner's [C++ SGP4 library](http://www.danrw.com/sgp4-satellite.php), based on ["Revisiting Spacetrack Report #3"](http://www.celestrak.com/publications/AIAA/2006-6753/), by David Vallado, Paul Crawford, Richard Hujsak, and [T.S. Kelso](http://www.celestrak.com/webmaster.asp). [Two-line element](http://celestrak.com/NORAD/elements/) (TLE) sets of orbit parameters are read as input. [Shapelib 1.3.0b3](http://shapelib.maptools.org/) is used to generate the shapefiles. 

## Features

Ground Track Generator can represent each step of the ground track as points or line segments. The step size is determined by a user-specified time interval. The extent of the ground track is controlled by a start time (absolute or relative to the TLE epoch) and either an end time or a step count. A variety of attributes can be output for each step, including elevation and azimuth as viewed from an optionally-specified ground observer. Here, intervals of an orbit with positive elevation as viewed from the northeast US are symbolized with increasingly large circles:

![trace with observer attributes](https://github.com/anoved/Ground-Track-Generator/raw/master/test/images/elevation-trace.png)

## Reference

If you run `gtg --help`, the program lists options it understands. At present this message is unfortunately perhaps more useful as a reference for the programmer than as a guide for users.

	Ground Track Generator 0.1
	usage: gtg [OPTIONS] [TLE [TLE ...]]
	
	OPTIONS:
	
		--attributes/-a standard | all | ATTRIBUTE [ATTRIBUTE ...]
			By default, no attributes are output.
				all       - Output all attributes. Some require an --observer.
				standard  - All attributes except those which require an observer.
			Alternatively, list one or more of the following ATTRIBUTE names:
				time      - Step timestamp in YYYY-MM-DD HH:MM:SS.SSSSSS UTC
				unixtime  - Step timestamp in seconds since 0:0:0 UTC 1 Jan 1970.
				mfe       - Relative timestamp in minutes from epoch.
				latitude  - Geodetic latitude of satellite position.
				longitude - Geodetic longitude of satellite position.
				altitude  - Altitude of satellite in km.
				velocity  - Magnitude of satellite velocity in km/s.
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
		
		--prj
			Write a .prj file specifying the geodetic reference system of coordinate
			output (WGS-72) to the same base path as the output shapefile.
		
		--interval/-l DURATION
			Step interval. Duration format is a number followed by s, m, h, or d,
			indicating the unit (seconds, minutes, hours, or days, respectively).
			
		--steps/-n STEPS
			Number of steps to output. Defaults to 100. Ignored if --end is given.
		
		--start/-s now | epoch | TIME | UNIXTIME
			Timestamp for first step of output. Subsequent steps are output at
			uniform intervals specified by --interval and --unit.
				now[OFFSET]   - Current time, with optional offset.
				epoch[OFFSET] - Default. TLE reference time, with optional offset.
				TIME          - Time in "YYYY-MM-DD HH:MM:SS.SSSSSS UTC" format.
				UNIXTIME      - Time in seconds since 0:0:0 UTC 1 Jan 1970.
			OFFSET format is a number followed by s, m, h, or d, indicating the
			offset unit (seconds, minutes, hours, or days, respectively).
		
		--end/e now | epoch | TIME | UNIXTIME
			If specified, trace is output from --start to no later than --end. If
			not specified, trace is output for the specified number of --steps.
			Arguments interpreted the same as --start. Must be later than that
			--start time. Interval between --start and --end must be greater than
			the step interval defined by --interval and --unit.
		
		--forceend
			Causes a final feature to be output exactly at --end time, regardless of
			interval. Has no effect if --end is not specified.
		
		--features/-f point | line
			Specify whether to output points (default) or line segment features for
			each step. Attributes refer to the starting point of line features.
		
		--split/-d
			If generating line --features, split any lines that cross the 180th
			meridian into east and west hemisphere segments. IMPORTANT: This is
			intended only as a cosmetic convenience; the latitude of the split point
			is not determined with the same geodetic precision as the step points.
			Not enabled by default.
		
		--observer/-g LATITUDE LONGITUDE [ALTITUDE]
			Specify the surface location of an observer (optional ALTITUDE in km).
			Some --attributes require an observer to be defined. Default: none.
		
		--tle/-t TEXT
			Load the first two-line element set found in TEXT.
		
		--input/-i PATH
			Load the first two-line element set found in the file at PATH.
		
		--output/-o PATH | DIRECTORY
			If a single two-line element set is loaded, specify the base PATH of the
			output (defaults to an identifier from the two-line element set). If
			multiple two-line element sets are loaded, specify the directory in
			which to write output files (defaults to current working directory).
		
		--prefix/-p PREFIX
			If specified, PREFIX is prepended to the base name identifier, unless
			there is only one two-line element set and an --output PATH is given.
		
		--suffix/-x SUFFIX
			If specified, SUFFIX is appended to the base name identifier, unless
			there is only one two-line element set and an --output PATH is given.
		
		--verbose
			Print status messages (including coordinates and attribute values).
		
		--help/-?
			Display this usage message.
		
		--version/-v
			Display the program version.
	
	Loading Two-Line Element Sets:
		Multiple two-line element sets can be loaded with multiple TLE arguments or
		--tle or --input options. If multiple two-line element sets are loaded, a
		ground track shapefile will be output for each set. If no TLE arguments or
		--tle or --input options are specified, the program will attempt to load the
		first two-line element set read from standard input.
	
	Output Filenames:
		Each output shapefile consists of three files with the same base name and
		the following file extensions: .shp, .shx, and .dbf. (If --prj is specified,
		a fourth file with the same basename and extension .prj is also output.)
		
		If multiple two-line element sets are loaded, the base name is constructed
		from this format: <DIRECTORY>/<PREFIX>TLEID<SUFFIX>, where TLEID is an
		identifier (NORAD number) read from the two-line element set.
		
		If a single two-line element set is loaded and an --output PATH is
		specified, the base name is simply PATH.
		
		If a single two-line element set is loaded and no --output PATH is
		specified, the base name takes this format: <PREFIX>TLEID<SUFFIX>.
