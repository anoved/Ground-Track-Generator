/*
 * gtgutil
 *
 * Provides utility functions for displaying error, status, and help messages.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "gtg.h"

#include "gtgutil.h"

namespace gtgutil {
	bool verbose = false;
}

/*
 * Print an error message to stderr and exit with failure status.
 * Message may include format specifiers with additional variable arguments.
 */
void FailDetail(const char *file, int line, const char *errorString, ...) {
	va_list arglist;
	va_start(arglist, errorString);
	fprintf(stderr, "%s:%s:%d: ", _GTG_PROGRAM_, file, line);
	vfprintf(stderr, errorString, arglist);
	va_end(arglist);
	exit(EXIT_FAILURE);
}

/*
 * Toggle verbose mode.
 */
void SetVerbosity(bool verbose)
{
	gtgutil::verbose = verbose;
}

/*
 * Print a diagnostic message to stdout (if verbose mode is enabled).
 * Message may include format specifiers with additional variable arguments.
 */
void Note(const char *noteString, ...) {
	va_list arglist;
	if (gtgutil::verbose) {
		va_start(arglist, noteString);
		vprintf(noteString, arglist);
		va_end(arglist);
	}
}

/*
 * Display program name and version and quit successfully.
 */
void ShowVersion(void)
{
	printf("%s %s\n", _GTG_NAME_, _GTG_VERSION_);
	exit(EXIT_SUCCESS);
}

/*
 * Display a concise usage message and quit successfully.
 */
void ShowHelp(void)
{
	printf("%s %s\n", _GTG_NAME_, _GTG_VERSION_);
	printf("usage: %s [OPTIONS] [TLE [TLE ...]]\n", _GTG_PROGRAM_);
	printf("\n");
	printf("OPTIONS:\n");
	printf("\n");
	printf("    --attributes/-a standard | all | ATTRIBUTE [ATTRIBUTE ...]\n");
	printf("        By default, no attributes are output.\n");
	printf("            all       - Output all attributes. Some require an --observer.\n");
	printf("            standard  - All attributes except those which require an observer.\n");
	printf("        Alternatively, list one or more of the following ATTRIBUTE names:\n");
	printf("            time      - Step timestamp in YYYY-MM-DD HH:MM:SS.SSSSSS UTC\n");
	printf("            unixtime  - Step timestamp in seconds since 0:0:0 UTC 1 Jan 1970.\n");
	printf("            mfe       - Relative timestamp in minutes from epoch.\n");
	printf("            latitude  - Geodetic latitude of satellite position.\n");
	printf("            longitude - Geodetic longitude of satellite position.\n");
	printf("            altitude  - Altitude of satellite in km.\n");
	printf("            velocity  - Magnitude of satellite velocity in km/s.\n");
	printf("            xposition - Earth Centered Inertial (ECI) x position in km.\n");
	printf("            yposition - Satellite ECI y position in km.\n");
	printf("            zposition - Satellite ECI z position in km.\n");
	printf("            xvelocity - Satellite ECI x velocity in km/s.\n");
	printf("            yvelocity - Satellite ECI y velocity in km/s.\n");
	printf("            zvelocity - Satellite ECI z velocity in km/s.\n");
	printf("            range     - Range to satellite from observer in km.\n");
	printf("            rate      - Rate of satellite range from observer in km/s.\n");
	printf("            elevation - Elevation to satellite from observer.\n");
	printf("            azimuth   - Azimuth to satellite from observer.\n");
	printf("    \n");
	printf("    --prj\n");
	printf("        Write a .prj file specifying the geodetic reference system of coordinate\n");
	printf("        output (WGS-72) to the same base path as the output shapefile.\n");
	printf("    \n");
	printf("    --interval/-l DURATION\n");
	printf("        Step interval. Duration format is a number followed by s, m, h, or d,\n");
	printf("        indicating the unit (seconds, minutes, hours, or days, respectively).\n");
	printf("        \n");
	printf("    --steps/-n STEPS\n");
	printf("        Number of steps to output. Defaults to 100. Ignored if --end is given.\n");
	printf("    \n");
	printf("    --start/-s now | epoch | TIME | UNIXTIME\n");
	printf("        Timestamp for first step of output. Subsequent steps are output at\n");
	printf("        uniform intervals specified by --interval and --unit.\n");
	printf("            now[OFFSET]   - Current time, with optional offset.\n");
	printf("            epoch[OFFSET] - Default. TLE reference time, with optional offset.\n");
	printf("            TIME          - Time in \"YYYY-MM-DD HH:MM:SS.SSSSSS UTC\" format.\n");
	printf("            UNIXTIME      - Time in seconds since 0:0:0 UTC 1 Jan 1970.\n");
	printf("    	OFFSET format is a number followed by s, m, h, or d, indicating the\n");
	printf("    	offset unit (seconds, minutes, hours, or days, respectively).\n");
	printf("    \n");
	printf("    --end/e now | epoch | TIME | UNIXTIME\n");
	printf("        If specified, trace is output from --start to --end time. If not\n");
	printf("        specified, trace is output for the specified number of --steps.\n");
	printf("        Arguments interpreted the same as --start. Must be later than that\n");
	printf("        --start time. Interval between --start and --end must be greater than\n");
	printf("        the step interval defined by --interval and --unit.\n");
	printf("    \n");
	printf("    --features/-f point | line\n");
	printf("        Specify whether to output points (default) or line segment features for\n");
	printf("        each step. Attributes refer to the starting point of line features.\n");
	printf("    \n");
	printf("    --split/-d\n");
	printf("        If generating line --features, split any lines that cross the 180th\n");
	printf("        meridian into east and west hemisphere segments. IMPORTANT: This is\n");
	printf("        intended only as a cosmetic convenience; the latitude of the split point\n");
	printf("        is not determined with the same geodetic precision as the step points.\n");
	printf("        Not enabled by default.\n");
	printf("    \n");
	printf("    --observer/-g LATITUDE LONGITUDE [ALTITUDE]\n");
	printf("        Specify the surface location of an observer (optional ALTITUDE in km).\n");
	printf("        Some --attributes require an observer to be defined. Default: none.\n");
	printf("    \n");
	printf("    --tle/-t TEXT\n");
	printf("        Load the first two-line element set found in TEXT.\n");
	printf("    \n");
	printf("    --input/-i PATH\n");
	printf("        Load the first two-line element set found in the file at PATH.\n");
	printf("    \n");
	printf("    --output/-o PATH | DIRECTORY\n");
	printf("        If a single two-line element set is loaded, specify the base PATH of the\n");
	printf("        output (defaults to an identifier from the two-line element set). If\n");
	printf("        multiple two-line element sets are loaded, specify the directory in\n");
	printf("        which to write output files (defaults to current working directory).\n");
	printf("    \n");
	printf("    --prefix/-p PREFIX\n");
	printf("        If specified, PREFIX is prepended to the base name identifier, unless\n");
	printf("        there is only one two-line element set and an --output PATH is given.\n");
	printf("    \n");
	printf("    --suffix/-x SUFFIX\n");
	printf("        If specified, SUFFIX is appended to the base name identifier, unless\n");
	printf("        there is only one two-line element set and an --output PATH is given.\n");
	printf("    \n");
	printf("    --verbose\n");
	printf("        Print status messages (including coordinates and attribute values).\n");
	printf("    \n");
	printf("    --help/-?\n");
	printf("        Display this usage message.\n");
	printf("    \n");
	printf("    --version/-v\n");
	printf("        Display the program version.\n");
	printf("\n");
	printf("Loading Two-Line Element Sets:\n");
	printf("    Multiple two-line element sets can be loaded with multiple TLE arguments or\n");
	printf("    --tle or --input options. If multiple two-line element sets are loaded, a\n");
	printf("    ground track shapefile will be output for each set. If no TLE arguments or\n");
	printf("    --tle or --input options are specified, the program will attempt to load the\n");
	printf("    first two-line element set read from standard input.\n");
	printf("\n");
	printf("Output Filenames:\n");
	printf("    Each output shapefile consists of three files with the same base name and\n");
	printf("    the following file extensions: .shp, .shx, and .dbf. (If --prj is specified,\n");
	printf("    a fourth file with the same basename and extension .prj is also output.)\n");
	printf("    \n");
	printf("    If multiple two-line element sets are loaded, the base name is constructed\n");
	printf("    from this format: <DIRECTORY>/<PREFIX>TLEID<SUFFIX>, where TLEID is an\n");
	printf("    identifier (NORAD number) read from the two-line element set.\n");
	printf("    \n");
	printf("    If a single two-line element set is loaded and an --output PATH is\n");
	printf("    specified, the base name is simply PATH.\n");
	printf("    \n");
	printf("    If a single two-line element set is loaded and no --output PATH is\n");
	printf("    specified, the base name takes this format: <PREFIX>TLEID<SUFFIX>.\n");
	exit(EXIT_SUCCESS);
}

