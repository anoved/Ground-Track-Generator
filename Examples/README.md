# Examples

Ground Track Generator (`gtg`) reads observed information about satellite orbits from [two-line element sets](http://www.celestrak.com/columns/v04n03/). It feeds this information into a [simplified perturbation model](https://en.wikipedia.org/wiki/Simplified_perturbations_models) to plot a ground track of the satellite's position for a specified period. This document demonstrates how to use `gtg` through a series of examples.

We'll start with a current two-line element set for [GeoEye-1](http://www.geoeye.com/CorpSite/products-and-services/imagery-sources/Default.aspx#geoeye1). At [CelesTrak](http://www.celestrak.com/NORAD/elements/), it is listed among [Earth Resources](http://www.celestrak.com/NORAD/elements/resource.txt) satellites. For this example, we only want to deal with one satellite, so we'll copy the relevant lines and save them as `geoeye1.tle`:

	GEOEYE 1                
	1 33331U 08042A   12091.09077403  .00000328  00000-0  70846-4 0  9565
	2 33331  98.1132 165.3484 0011197 115.0161 245.2220 14.64451624190457

We can specify two-line element sets in a couple ways, but for clarity we will use the `--input` option throughout this example. We'll also use the `--output` option to explicitly name the output files. (Otherwise, `gtg` would use the satellite number, `33331`, as the default name.) In each case here, the output will consist of a set of four files (.shp, .shx, .dbf, and .prj) which share the specified base name and comprise a shapefile.

## 1

We'll start with a very simple case:

	gtg --input geoeye1.tle --output example1

In this case, the output consists of a single point representing the location of GeoEye-1 at the "epoch" (reference date) specified in the two-line element set. In `geoeye1.tle`, the epoch is encoded as `12091.09077403`. The first [two digits](http://www.celestrak.com/columns/v04n03/#FAQ04) specify the year, 2012, and the remaining digits specify the day of the year, 91.09077403 (at some point on March XX).

Here's the point. It's over South America:

![Example 1 Illustration](https://github.com/anoved/Ground-Track-Generator/blob/master/Examples/Images/1.png?raw=true)

## 2

One point may be useful, but now let's generate an actual ground track:

	gtg --input geoeye1.tle --output example2 --start epoch --end epoch+90m

For clarity, we've explicitly specified the default starting time. We've also added an end time 90 minutes after the epoch. By specifying an end time, we've told `gtg` that we want it to generate a ground track of GeoEye-1's path over that period. Here's what it looks like:

![Example 2 Illustration](https://github.com/anoved/Ground-Track-Generator/blob/master/Examples/Images/2.png?raw=true)

A series of 90 points has produced, starting with the same point we saw in example 1.

## 3

From example 2, you can deduce that, by default, `gtg` generates ground track points at one-minute intervals. We can use the `--interval` option to control this behavior:

	gtg --input geoeye1.tle --output example3 --start epoch --end epoch+90m --interval 30s

By halving the interval to 30 seconds, we've doubled the number of output points to 180, resulting in a "smoother" ground track for the same period:

![Example 3 Illustration](https://github.com/anoved/Ground-Track-Generator/blob/master/Examples/Images/3.png?raw=true)

## 4

As an alternative to specifying an `--end` time, you can state how many `--steps` to generate.

	gtg --input geoeye1.tle --output example4 --start epoch --interval 30s --steps 180

This combination of options produces the same ground track as example 3:

![Example 4 Illustration](https://github.com/anoved/Ground-Track-Generator/blob/master/Examples/Images/4.png?raw=true)

## 5

Plotting the ground track points is useful, but next let's put `gtg` to work telling us more about the situation at each point.

	gtg --input geoeye1.tle --output example5 --start epoch --interval 30s --steps 180 --attributes standard

We've added the `--attributes` option to tell `gtg` that we want it to add some fields to the output shapefile's attribute table. The `standard` argument is a shortcut that means we want all the information `gtg` can provide without any additional setup. Here's a peek at the attribute table. The ground track point corresponding to the selected record is highlighted in yellow:

![Example 5 Illustration](https://github.com/anoved/Ground-Track-Generator/blob/master/Examples/Images/5.png?raw=true)

The `FID` field is an index number. `time` is the [UTC](https://en.wikipedia.org/wiki/Coordinated_Universal_Time) timestamp of the point. `unixtime` is the number of seconds since 00:00:00 UTC, 1 January 1970 (a [common format](https://en.wikipedia.org/wiki/Unix_time) for recording event times). `mfe` is "minutes from epoch", the relative offset between the two-line element set's epoch and point's time (the orbital perturbation model is most accurate closest to 0). `altitude` is GeoEye-1's altitude in kilometers. `velocity` is actually the magnitude of the satellite's velocity relative to the ["Earth-Centered Inertial" (ECI) coordinate system](http://www.celestrak.com/columns/v02n01/), expressed in kilometers per second. `xposition`, `yposition`, and `zposition` are the ECI coordinates of the satellite in kilometers. Likewise, ` xvelocity`, `yvelocity`, and `zvelocity` are the components of the satellite's ECI velocity, in kilometers per second.

## 6

Now let's specify the location of a ground `--observer` and ask for `all` attributes.

	gtg --input geoeye1.tle --output example6 --start epoch --interval 30s --steps 180 --attributes all --observer 42.102222 -75.911667

In this case, we've specified an observer in the northeast US (represented by the blue diamond in the image below). For each point, `gtg` now outputs a few attributes relative to this location: `range` is the distance is kilometers, `rate` is the rate of range change in kilometers per second (negative values suggest the satellite is approaching; positive values suggest it is receding), and `azimuth` and `elevation` specify where the observer should look (in the [horizontal coordinate system](http://en.wikipedia.org/wiki/Horizontal_coordinate_system) - degrees clockwise from north and degrees above the horizon, respectively) to observe the satellite.

![Example 6 Illustration](https://github.com/anoved/Ground-Track-Generator/blob/master/Examples/Images/6.png?raw=true)

In this image, the ground track points are color coded by `rate` - the GeoEye-1 is coming closer to the observer when the points are blue and getting further when the points are red. The attributes of the yellow highlighted points are selected in the attribute table. Its `azimuth` is 44.6 - very nearly to the northeast of the observer.

## 7

In this last example, we'll represent the ground track as a continuous line instead of a series of points.

	gtg --input geoeye1.tle --output example7 --start epoch --interval 30s --steps 180 --features line --split

The `--features` option is used to specify that we want to output `line` features (the default is `point`). The `--split` option splits line segments that cross the 180th meridian into two line segments, one with longitudes <= 180 and the other with longitudes >= -180. (Otherwise, many mapping programs may misinterpret the direction of the line segment, as illustrated by the dotted line on the map below.)

![Example 7 Illustration](https://github.com/anoved/Ground-Track-Generator/blob/master/Examples/Images/7.png?raw=true)

Note that the behavior of `--split` is untested for large intervals (eg, cases where consecutive points skip hemispheres or orbits). However, `line` mode is probably not appropriate in these cases anyway, since small intervals are required to produce a smooth path.

---

## Reference

For more details on the options `gtg` understands, you can always run:

	gtg --help

Here is the full text of the help message:

	Ground Track Generator 1.0
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
	
