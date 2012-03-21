#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gtg.h"
#include "gtgutil.h"
#include "gtgtrace.h"

struct configuration cfg;

int main(int argc, char *argv[])
{
	int opt = 0;
	int longIndex = 0;
	char **pos_argv;
	int pos_argc = 0;
	
	/* Initialize default configuration */
	cfg.start = NULL; /* NULL start implies epoch start time */
	cfg.end = NULL;
	cfg.unit = minutes;
	cfg.interval = 1.0;
	cfg.steps = 100;
	cfg.tleText = NULL;
	cfg.tlePath = NULL;
	cfg.shpPath = NULL;
	cfg.format = point;
	cfg.verbose = 0;
	
	/* Suppress getopt_long from printing its own error/warning messages */
	opterr = 0;

	/* Expected arguments for getopt_long */
	static const char *optString = "e:f:?i:l:o:s:n:t:u:v";
	static const struct option longOpts[] = {
			{"end", required_argument, NULL, 'e'},
			{"format", required_argument, NULL, 'f'},
			{"help", no_argument, NULL, '?'},
			{"input", required_argument, NULL, 'i'},
			{"interval", required_argument, NULL, 'l'},
			{"output", required_argument, NULL, 'o'},
			{"start", required_argument, NULL, 's'},
			{"steps", required_argument, NULL, 'n'},
			{"tle", required_argument, NULL, 't'},
			{"unit", required_argument, NULL, 'u'},
			{"verbose", no_argument, &cfg.verbose, 1},
			{"version", no_argument, NULL, 'v'},
			{NULL, no_argument, NULL, 0}
	};
	
	/* Store command line arguments and perform some preliminary validation */
	while(-1 != (opt = getopt_long_only(argc, argv, optString, longOpts, &longIndex))) {
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
					cfg.unit = seconds;
				} else if (0 == strcmp("minutes", optarg)) {
					cfg.unit = minutes;
				} else if (0 == strcmp("hours", optarg)) {
					cfg.unit = hours;
				} else if (0 == strcmp("days", optarg)) {
					cfg.unit = days;
				} else {
					Fail("Invalid interval unit (should be one of seconds, minutes, hours, or days).\n");
				}
				break;
			
			case 'l':
				/* Interval length */
				/* Argument format: floating point number */
				if (1 != sscanf(optarg, "%lf", &cfg.interval)) {
					Fail("Invalid interval length (should be positive floating point number).\n");
				}
				break;
			
			case 'n':
				/* Steps */
				/* Argument format: integer >= 1 */
				if (1 != sscanf(optarg, "%d", &cfg.steps)) {
					Fail("Invalid feature count (should be positive integer).\n");
				}
				break;
			
			case 't':
				/* TLE text */
				/* Argument format: text to be parsed for TLE, ala runtest */
				if (NULL != cfg.tleText) {
					Fail("Extraneous TLE text argument.\n");
				}
				if (NULL != cfg.tlePath) {
					Fail("Extraneous TLE text argument (input TLE file already specified).\n");
				}
				cfg.tleText = optarg;
				break;
			
			case 'i':
				/* Input file */
				/* Argument format: path to file to read for TLE, ala runtest */
				if (NULL != cfg.tlePath) {
					Fail("Extraneous input TLE file path argument.\n");
				}
				if (NULL != cfg.tleText) {
					Fail("Extraneous input TLE file path argument (TLE text already specified).\n");
				}
				cfg.tlePath = optarg;
				break;
			
			case 'o':
				/* Output file */
				/* Argument format: path to output shapefile basename */
				if (NULL != cfg.shpPath) {
					Fail("Extraneous output shapefile base name argument.\n");
				}
				cfg.shpPath = optarg;
				break;
			
			case 'f':
				/* Output format */
				/* Accepted argument values: point, line */
				if (0 == strcmp("point", optarg)) {
					cfg.format = point;
				} else if (0 == strcmp("line", optarg)) {
					cfg.format = line;
				} else {
					Fail("Invalid output format (should be point or line).\n");
				}
				break;
			
			case 'v':
				/* Version information */
				ShowVersion();
				break;
				
			case 0:
				/* (options that just store val in flag, such as --verbose) */
				/* (exact option indicated by longOpts[longIndex].name) */
				break;
			
			case '?':
			default:
				/* Help / usage OR unrecognized options */
				if ((0 == strcmp("help", longOpts[longIndex].name))
						|| (0 == strcmp("-?", argv[optind - 1]))) {
					ShowHelp();
				} else {
					Fail("Unrecognized option: %s (try --help)\n", argv[optind - 1]);
				}
				break;
		}
	}
	
	/* Positional arguments */
	pos_argv = argv + optind;
	pos_argc = argc - optind;
	
	/* Determine output path */
	if (NULL == cfg.shpPath) {
		if (1 == pos_argc) {
			cfg.shpPath = pos_argv[0];
		} else {
			Fail("no output shapefile specified\n");
		}
	} else if (0 != pos_argc) {
		Fail("extra output shapefile specified (--output already specified as %s).\n", cfg.shpPath);
	}
		
	/* Determine whether output is constrained by end time or feature count */
	if (NULL == cfg.end) {
		/* If no end is specified, output the specified number of features. */
		if (0 >= cfg.steps) {
			Fail("Feature count for line output must be a positive integer.\n");
		}
	} else {
		/* If an end point IS specified, ignore feature count. */
		cfg.steps = 0; 
	}
	
	/* get dis party started */
	StartGroundTrack();
	
	return EXIT_SUCCESS;
}
