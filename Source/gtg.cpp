#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gtg.h"
#include "gtgutil.h"
#include "gtgtrace.h"

struct configuration cfg;

void ShowVersion(void)
{
	printf("%s %s\n", _GTG_NAME_, _GTG_VERSION_);
	exit(EXIT_SUCCESS);
}

void ShowHelp(void)
{
	printf("%s %s\n", _GTG_NAME_, _GTG_VERSION_);
	printf("usage: gtg [--start] [--end] [--interval_unit] [--interval_length] [--tle]\n");
	printf("           [--feature_count] [--input] [--output] [--format] [output]\n");
	printf("Web page: <https://github.com/anoved/Ground-Track-Generator>\n");
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
	int opt = 0;
	int longIndex = 0;
	char **pos_argv;
	int pos_argc = 0;
	
	/* Initialize default configuration */
	cfg.start = NULL; /* NULL start implies epoch start time */
	cfg.end = NULL;
	cfg.interval_units = minutes;
	cfg.interval_length = 1.0;
	cfg.feature_count = 100;
	cfg.tleText = NULL;
	cfg.inputTlePath = NULL;
	cfg.outputShpBasepath = NULL;
	cfg.format = point;
	
	/* Suppress getopt_long from printing its own error/warning messages */
	opterr = 0;

	/* Expected arguments for getopt_long */
	static const char *optString = "s:e:u:l:c:n:t:i:o:f:v?";
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
			{"version", no_argument, NULL, 'v'},
			{"help", no_argument, NULL, '?'},
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
			
			case '?':
				/* Help / Usage */
				ShowHelp();
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
		
	/* Determine whether output is constrained by end time or feature count */
	if (NULL == cfg.end) {
		/* If no end is specified, output the specified number of features. */
		if (0 >= cfg.feature_count) {
			Fail("Feature count for line output must be a positive integer.\n");
		}
	} else {
		/* If an end point IS specified, ignore feature count. */
		cfg.feature_count = 0; 
	}
	
	/* get dis party started */
	StartGroundTrack();
	
	return EXIT_SUCCESS;
}
