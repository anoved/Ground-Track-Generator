#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* Intermediate structure to help keep command line parameters organized */
enum interval_unit_type {
	seconds,
	minutes,
	hours,
	days
};
enum output_format_type {
	multipoint,
	line
};
struct configuration {
	char *start;
	char *end;
	enum interval_unit_type interval_units;
	double interval_length;
	int feature_count;
	char *tleText;
	char *inputTlePath;
	char *outputShpBasepath;
	enum output_format_type format;
} cfg;

/* Expected arguments for getopt_long */
static const char *optString = "s:e:u:l:c:n:t:i:o:f:";
static const struct option longOpts[] = {
		{"start", required_argument, NULL, 's'},
		{"end", required_argument, NULL, 'e'},
		{"interval_unit", required_argument, NULL, 'u'},
		{"interval_length", required_argument, NULL, 'l'},
		{"feature_count", required_argument, NULL, 'n'},
		{"tle", required_argument, NULL, 't'},
		{"input", required_argument, NULL, 'i'},
		{"output", required_argument, NULL, 'o'},
		{"format", required_argument, NULL, 'f'},
		{NULL, no_argument, NULL, 0}
};

/* Print an error message to stderr and exit with failure status. */
void Fail(const char *errorString, ...) {
	va_list arglist;
	va_start(arglist, errorString);
	fprintf(stderr, "ERROR:\n");
	vfprintf(stderr, errorString, arglist);
	va_end(arglist);
	exit(EXIT_FAILURE);
}

/* Print a diagnostic message. We could rewrite these to a log file. */
void Note(const char *noteString, ...) {
	va_list arglist;
	va_start(arglist, noteString);
	vprintf(noteString, arglist);
	va_end(arglist);
}

int main(int argc, char *argv[])
{
	int opt = 0;
	int longIndex = 0;
	char **pos_argv;
	int pos_argc = 0;
	
	/* Initialize default configuration */
	cfg.start = NULL;
	cfg.end = NULL;
	cfg.interval_units = hours;
	cfg.interval_length = 1.0;
	cfg.feature_count = 24;
	cfg.tleText = NULL;
	cfg.inputTlePath = NULL;
	cfg.outputShpBasepath = NULL;
	cfg.format = line;
	
	/* Suppress getopt_long from printing its own error/warning messages */
	opterr = 0;
	
	/* Store command line arguments and perform some preliminary validation */
	while(-1 != (opt = getopt_long(argc, argv, optString, longOpts, &longIndex))) {
		switch(opt) {
			
			case 's':
				/* Start */
				if(NULL != cfg.start) {
					Fail("Extraneous trace start time argument.");
				}
				cfg.start = optarg;
				break;
			
			case 'e':
				/* End */
				if (NULL != cfg.end) {
					Fail("Extraneous trace end time argument.");
				}
				cfg.end = optarg;
				break;
			
			case 'u':
				/* Interval units */
				/* Accepted argument values: seconds, minutes, hours, days */
				if (0 == strcmp("seconds", optarg)) {
					cfg.interval_units = seconds;
				} else if (0 == strcmp("minutes", optarg)) {
					cfg.interval_units = minutes;
				} else if (0 == strcmp("hours", optarg)) {
					cfg.interval_units = hours;
				} else if (0 == strcmp("days", optarg)) {
					cfg.interval_units = days;
				} else {
					Fail("Invalid interval unit (should be one of seconds, minutes, hours, or days).\n");
				}
				break;
			
			case 'l':
				/* Interval length */
				/* Argument format: floating point number */
				if (1 != sscanf(optarg, "%lf", &cfg.interval_length)) {
					Fail("Invalid interval length (should be positive floating point number).\n");
				}
				break;
			
			case 'n':
				/* Feature count */
				/* Argument format: integer >= 1 */
				if (1 != sscanf(optarg, "%d", &cfg.feature_count)) {
					Fail("Invalid feature count (should be positive integer).\n");
				}
				break;
			
			case 't':
				/* TLE text */
				/* Argument format: text to be parsed for TLE, ala runtest */
				if (NULL != cfg.tleText) {
					Fail("Extraneous TLE text argument.\n");
				}
				if (NULL != cfg.inputTlePath) {
					Fail("Extraneous TLE text argument (input TLE file already specified).\n");
				}
				cfg.tleText = optarg;
				break;
			
			case 'i':
				/* Input file */
				/* Argument format: path to file to read for TLE, ala runtest */
				if (NULL != cfg.inputTlePath) {
					Fail("Extraneous input TLE file path argument.\n");
				}
				if (NULL != cfg.tleText) {
					Fail("Extraneous input TLE file path argument (TLE text already specified).\n");
				}
				cfg.inputTlePath = optarg;
				break;
			
			case 'o':
				/* Output file */
				/* Argument format: path to output shapefile basename */
				if (NULL != cfg.outputShpBasepath) {
					Fail("Extraneous output shapefile base name argument.\n");
				}
				cfg.outputShpBasepath = optarg;
				break;
			
			case 'f':
				/* Output format */
				/* Accepted argument values: multipoint, line */
				if (0 == strcmp("multipoint", optarg)) {
					cfg.format = multipoint;
				} else if (0 == strcmp("line", optarg)) {
					cfg.format = line;
				} else {
					Fail("Invalid output format (should be multipoint or line).\n");
				}
				break;
			
			default:
				Fail("Unrecognized option: %s\n", argv[optind - 1]);
				break;
		}
	}
	
	/* Positional arguments */
	pos_argv = argv + optind;
	pos_argc = argc - optind;

	/* Determine output path */
	if (NULL == cfg.outputShpBasepath) {
		if (1 == pos_argc) {
			cfg.outputShpBasepath = pos_argv[0];
		} else {
			Fail("Expected one additional argument to specify shapefile output path.\n");
		}
	} else if (0 != pos_argc) {
		Fail("Unexpected arguments (output path already specified).\n");
	}
	
	/* Confirm that a start time is specified */
	if (NULL == cfg.start) {
		Fail("No trace start time specified.\n");
	}
	
	/* Determine whether output is constrained by end time or feature count */
	if (NULL == cfg.end) {
		/* If no end is specified, output the specified number of features. */
		if (0 >= cfg.feature_count) {
			Fail("Feature count for line output must be a positive integer.\n");
		}
	} else {
		/* If an end point IS specified, ignore feature count. */
		cfg.feature_count = 0;
		
		/* Once start and end are parsed, validate that the intervening time
		 * span is greater than cfg.interval_length; if not, it's a Fail.
		 */ 
	}


	/** Configuration report... **/
	
	Note("Start: %s\n", cfg.start);
	if (cfg.end != NULL) {
		Note("End: %s\n", cfg.end);
	} else {
		Note("Feature count: %d\n", cfg.feature_count);
	}
	Note("Interval: %lf (%d)\n", cfg.interval_length, (int)cfg.interval_units);
	Note("Shapefile output will go to: %s\n", cfg.outputShpBasepath);	

	/* Report TLE source; use this conditional to direct actual TLE loading. */
	if ((NULL == cfg.tleText) and (NULL == cfg.inputTlePath)) {
		Note("TLEs will be read from stdin.\n");
	} else if (NULL != cfg.tleText) {
		Note("TLEs will be read from this argument:\n%s\n", cfg.tleText);
	} else if (NULL != cfg.inputTlePath) {
		Note("TLEs will be read from this file:\n%s\n", cfg.inputTlePath);
	}
	
	return EXIT_SUCCESS;
}
