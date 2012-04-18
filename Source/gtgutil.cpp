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
 * Print a non-fatal error message to stderr.
 * Message may include format specifiers with additional variable arguments.
 */
void Warn(const char *warnString, ...) {
	va_list arglist;
	va_start(arglist, warnString);
	vfprintf(stderr, warnString, arglist);
	va_end(arglist);
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
	printf("Ground Track Generator outputs GIS-compatible shapefiles containing point or\n");
	printf("line segment representations of the ground track of specified satellite orbits.\n");
	printf("The extent and resolution of the ground track is controlled by the TRACE OPTIONS\n");
	printf("described below. Orbit information is read from two-line element sets and the\n");
	printf("orbit is modelled with the SGP4/SDP4 simplified perturbation model.\n");
	printf("\n");
	printf("OPTIONS:\n");
	printf("\n");
	printf("  INPUT OPTIONS:\n");
	printf("\n");
	printf("    --tle/-t TEXT\n");
	printf("        Load two-line element sets directly from TEXT.\n");
	printf("\n");
	printf("    --input/-i PATH\n");
	printf("        Load two-line element sets from file PATH.\n");
	printf("\n");
	printf("    TLE [TLE ...]\n");
	printf("        Any command line arguments not interpreted as the options or arguments\n");
	printf("        described here are treated as the PATH to two-line element set files.\n");
	printf("\n");
	printf("    If no two-line element sets (TLEs) are loaded from the command line, gtg\n");
	printf("    will attempt to read two-line element sets from standard input. If multiple\n");
	printf("    TLEs are loaded, a separate shapefile will be output for each TLE.\n");
	printf("\n");
	printf("  OUTPUT OPTIONS:\n");
	printf("\n");
	printf("    --format/-m shapefile | csv\n");
	printf("        Select output format. Shapefile is default. If csv output is selected,\n");
	printf("        a comma-separated value is output, including id, latitude, longitude,\n");
	printf("        and any other specified attributes. If no --output argument is provided\n");
	printf("        in csv, data is written to standard output instead of a default file.\n");
	printf("\n");
	printf("    --header/-h\n");
	printf("    	Include a header row in csv output. No effect with any other --format.\n");
	printf("\n");
	printf("    --output/-o PATH | DIRECTORY\n");
	printf("        If a single two-line element set is loaded, specify the base PATH of the\n");
	printf("        output (defaults to the TLE identifier described below).\n");
	printf("        If multiple two-line element sets are loaded, specify the directory in\n");
	printf("        which to write output files (defaults to current working directory).\n");
	printf("\n");
	printf("    --prefix/-p PREFIX\n");
	printf("        If specified, PREFIX is prepended to the base name identifier, unless\n");
	printf("        there is only one two-line element set and an --output PATH is given.\n");
	printf("\n");
	printf("    --suffix/-x SUFFIX\n");
	printf("        If specified, SUFFIX is appended to the base name identifier, unless\n");
	printf("        there is only one two-line element set and an --output PATH is given.\n");
	printf("\n");
	printf("    --noprj\n");
	printf("        Suppress output of .prj \"projection\" file, which explicitly specifies\n");
	printf("        the geodetic reference system of the generated shapefile (WGS-72).\n");
	printf("\n");
	printf("    The default base name for output files is the [NORAD] satellite number\n");
	printf("    encoded in the second field of the first line of the two-line element set.\n");
	printf("\n");
	printf("  GEOMETRY OPTIONS:\n");
	printf("\n");
	printf("    --features/-f point | line\n");
	printf("        Specify whether to output points (the default) or line segment features\n");
	printf("        for each step. Attributes refer to the starting point of line features.\n");
	printf("\n");
	printf("    --split/-d\n");
	printf("        If generating line --features, split any lines that cross the 180th\n");
	printf("        meridian into east and west hemisphere segments. Disabled by default.\n");
	printf("      IMPORTANT: --split is intended as a cosmetic convenience only. The split\n");
	printf("        point latitude is not determined with the same precision as the trace.\n");
	printf("\n");
	printf("  ATTRIBUTE OPTIONS:\n");
	printf("\n");
	printf("    --attributes/-a all | standard | ATTRIBUTE [ATTRIBUTE ...]\n");
	printf("        By default, no attributes are output.\n");
	printf("            all       - Output all attributes. Some require an --observer.\n");
	printf("            standard  - All attributes except those which require an observer.\n");
	printf("        Alternatively, list one or more of the following ATTRIBUTE names:\n");
	printf("            time      - Step timestamp in YYYY-MM-DD HH:MM:SS.SSSSSS UTC\n");
	printf("            unixtime  - Step timestamp in seconds since 0:0:0 UTC 1 Jan 1970.\n");
	printf("            mfe       - Relative timestamp in minutes from epoch.\n");
	printf("            altitude  - Altitude of satellite in km.\n");
	printf("            velocity  - Magnitude of satellite velocity in km/s.\n");
	printf("            latitude  - Geodetic latitude of satellite position.\n");
	printf("            longitude - Geodetic longitude of satellite position.\n");
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
	printf("        Attributes are output in this order regardless of order specified.\n");
	printf("\n");
	printf("    --observer/-g LATITUDE LONGITUDE [ALTITUDE]\n");
	printf("        Specify the surface location of an observer (optional altitude in km).\n");
	printf("        Some --attributes require an observer to be defined. None by default.\n");
	printf("\n");
	printf("  TRACE OPTIONS:\n");
	printf("\n");
	printf("    --start/-s now | epoch | TIME | UNIXTIME\n");
	printf("        Timestamp for first step of output. Subsequent steps are output at\n");
	printf("        uniform time --intervals until --end time or --steps count is reached.\n");
	printf("            now[OFFSET]     - Current time, with optional offset.\n");
	printf("            epoch[OFFSET]   - TLE reference time, with optional offset. Default.\n");
	printf("            SECONDS[OFFSET] - Time in seconds since 0:0:0 UTC 1 Jan 1970.\n");
	printf("            TIMESTAMP       - Time in \"YYYY-MM-DD HH:MM:SS.SSSSSS UTC\" format.\n");
	printf("        OFFSET format is a +/- number followed by s, m, h, or d, indicating the\n");
	printf("        offset unit (seconds, minutes, hours, or days, respectively).\n");
	printf("\n");
	printf("    --end/e now | epoch | TIME | UNIXTIME\n");
	printf("        If specified, trace is output from --start to no later than --end. If\n");
	printf("        not specified, trace is output for the specified number of --steps.\n");
	printf("        Same argument format as --start. The --end time must be later than the\n");
	printf("        --start time. The time interval between --start and --end must be\n");
	printf("        greater than the step --interval.\n");
	printf("\n");
	printf("    --forceend\n");
	printf("        Causes a final feature to be output exactly at --end time, regardless of\n");
	printf("        interval. Has no effect if --end is not specified.\n");
	printf("\n");
	printf("    --steps/-n STEPS\n");
	printf("        Number of steps to output. Defaults to 1. Ignored if --end is given.\n");
	printf("\n");
	printf("    --interval/-l DURATION\n");
	printf("        Step interval. Duration format is a number followed by s, m, h, or d,\n");
	printf("        indicating the unit (seconds, minutes, hours, or days, respectively).\n");
	printf("\n");
	printf("  MISCELLANEOUS OPTIONS:\n");
	printf("\n");
	printf("    --verbose\n");
	printf("        Print status messages (including coordinates and attribute values).\n");
	printf("\n");
	printf("    --help/-?\n");
	printf("        Display this usage message.\n");
	printf("\n");
	printf("    --version/-v\n");
	printf("        Display the program version.\n");
	printf("\n");
	printf("CREDITS:\n");
	printf("\n");
	printf("    C++ SGP4 Satellite Library:\n");
	printf("    <http://www.danrw.com/sgp4-satellite.php>\n");
	printf("\n");
	printf("    Shapefile C Library:\n");
	printf("    <http://shapelib.maptools.org/>\n");
	printf("\n");
	printf("    Revisiting Spacetrack Report #3 (background reference and test cases):\n");
	printf("    <http://www.celestrak.com/publications/AIAA/2006-6753/>\n");
	printf("\n");
	exit(EXIT_SUCCESS);
}

