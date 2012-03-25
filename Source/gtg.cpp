#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <queue>

#include "Tle.h"
#include "Julian.h"

#include "gtg.h"
#include "gtgtle.h"
#include "gtgutil.h"
#include "gtgtrace.h"
#include "gtgattr.h"

int main(int argc, char *argv[])
{
	Julian now;
	int opt = 0;
	int longIndex = 0;
	bool has_observer = false;
	std::queue<Tle> tles;
	GTGConfiguration cfg;
	
	/* All attributes turned off by default */
	FlagAllAttributes(false);
	
	/* Initialize default configuration */
	cfg.start = NULL; /* NULL start implies epoch start time */
	cfg.end = NULL;
	cfg.unit = minutes;
	cfg.interval = 1.0;
	cfg.steps = 100;
	cfg.basepath = NULL;
	cfg.features = point;
	cfg.split = 0;
	cfg.obslat = 0;
	cfg.obslon = 0;
	cfg.obsalt = 0;
	cfg.prefix = NULL;
	cfg.suffix = NULL;
	cfg.prj = 0;
	cfg.single = false;
	
	/* Suppress getopt_long from printing its own error/warning messages */
	opterr = 0;

	/* Expected arguments for getopt_long */
	const char *optString = "a:d:e:f:?i:l:g:o:p:s:n:t:u:x:v";
	const struct option longOpts[] = {
			{"attributes", required_argument, NULL, 'a'},
			{"end", required_argument, NULL, 'e'},
			{"features", required_argument, NULL, 'f'},
			{"help", no_argument, NULL, '?'},
			{"input", required_argument, NULL, 'i'},
			{"interval", required_argument, NULL, 'l'},
			{"observer", required_argument, NULL, 'g'},
			{"output", required_argument, NULL, 'o'},
			{"prefix", required_argument, NULL, 'p'},
			{"prj", no_argument, &cfg.prj, 1},
			{"split", no_argument, NULL, 'd'},
			{"start", required_argument, NULL, 's'},
			{"steps", required_argument, NULL, 'n'},
			{"suffix", required_argument, NULL, 'x'},
			{"tle", required_argument, NULL, 't'},
			{"unit", required_argument, NULL, 'u'},
			{"verbose", no_argument, NULL, 0},
			{"version", no_argument, NULL, 'v'},
			{NULL, no_argument, NULL, 0}
	};
	
	/* Store command line arguments and perform some preliminary validation */
	while(-1 != (opt = getopt_long_only(argc, argv, optString, longOpts, &longIndex))) {
		switch(opt) {
			
			case 'p':
				/* Output prefix */
				cfg.prefix = optarg;
				break;
			
			case 'x':
				/* Output suffix (not including file extension) */
				cfg.suffix = optarg;
				break;
			
			case 'g':
				/* Ground observer */
				/* latitude and longitude arguments are required */
				
				if (1 != sscanf(optarg, "%lf", &cfg.obslat)) {
					Fail("cannot parse observer latitude: %s (should be number between -90 and 90)\n", optarg);
				}
				
				if (optind >= argc) {
					Fail("missing observer longitude\n");
				}
				
				if (1 != sscanf(argv[optind], "%lf", &cfg.obslon)) {
					Fail("cannot parse observer longitude: %s (should be number between -180 and 180)\n", argv[optind]);
				}
				
				has_observer = true;
				optind++;

				/* a third numeric argument, altitude (km), is optional */
				if (optind < argc) {
					if (1 == sscanf(argv[optind], "%lf", &cfg.obsalt)) {
						optind++;
					} // no complaint if we can't read it; leave it for getopt
				}					
				break;
			
			case 'd':
				/* Split line segments that cross the 180th meridian/dateline */
				cfg.split = 1;
				break;
			
			case 'a':
				/* Attributes */
				if (0 == strcmp("all", optarg)) {
					FlagAllAttributes(true);
				} else if (0 == strcmp("standard", optarg)) {
					FlagAllAttributes(true, true);
				} else {
				
					/* first attribute argument is required */
					if (not EnableAttribute(optarg)) {
						Fail("invalid attribute: %s\n", optarg);
					}
					
					/* subsequent attribute arguments, if present, are optional */
					/* if a subsequent argument doesn't look like an attribute, */
					/* just return control to getopt to handle as another opt */
					while (optind < argc) {
						if (EnableAttribute(argv[optind])) {
							optind++;
						} else {
							break;
						}
					}
				}				
				break;
			
			case 's':
				/* Start */
				if(NULL != cfg.start) {
					Fail("start already specified: %s\n", cfg.start);
				}
				cfg.start = optarg;
				break;
			
			case 'e':
				/* End */
				if (NULL != cfg.end) {
					Fail("end already specified: %s\n", cfg.end);
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
					Fail("invalid unit: %s (should be seconds, minutes, hours, or days)\n", optarg);
				}
				break;
			
			case 'l':
				/* Interval length */
				/* Argument format: positive floating point number */
				if (1 != sscanf(optarg, "%lf", &cfg.interval)) {
					Fail("cannot parse interval: %s (should be positive number)\n", optarg);
				}
				if (cfg.interval <= 0.0) {
					Fail("invalid interval: %s (should be positive number)\n", cfg.interval);
				}
				break;
			
			case 'n':
				/* Steps */
				/* Argument format: integer >= 1 */
				if (1 != sscanf(optarg, "%d", &cfg.steps)) {
					Fail("cannot parse steps: %s (should be positive integer)\n", optarg);
				}
				if (cfg.steps <= 0) {
					Fail("invalid steps: %s (should be positive integer)\n", cfg.steps);
				}
				break;
			
			case 't':
				/* TLE text */
				/* Argument format: text to be parsed for TLE, ala runtest */
				tles.push(ReadTleFromBuffer(optarg));
				break;
			
			case 'i':
				/* Input file */
				/* Argument format: path to file to read for TLE, ala runtest */
				tles.push(ReadTleFromPath(optarg));
				break;
			
			case 'o':
				/* Output directory */
				/* Argument format: path to output shapefile basename */
				if (NULL != cfg.basepath) {
					Fail("output directory already specified: %s\n", cfg.basepath);
				}
				cfg.basepath = optarg;
				break;
			
			case 'f':
				/* Feature type */
				/* Accepted argument values: point, line */
				if (0 == strcmp("point", optarg)) {
					cfg.features = point;
				} else if (0 == strcmp("line", optarg)) {
					cfg.features = line;
				} else {
					Fail("invalid feature type: %s (should be point or line)\n", optarg);
				}
				break;
			
			case 'v':
				/* Version information */
				ShowVersion();
				break;
				
			case 0:
				/* (options that just store val in flag, w/no short opt) */
				/* (exact option indicated by longOpts[longIndex].name) */
				if (0 == strcmp("verbose", longOpts[longIndex].name)) {
					SetVerbosity(true);
				}
				break;
			
			case '?':
			default:
				/* Help / usage OR unrecognized options */
				if ((0 == strcmp("help", longOpts[longIndex].name))
						|| (0 == strcmp("-?", argv[optind - 1]))) {
					ShowHelp();
				} else {
					Fail("unrecognized or incomplete option: %s (try --help)\n", argv[optind - 1]);
				}
				break;
		}
	}
	
	/* Positional arguments */
	argv += optind;
	argc -= optind;
	
	/* If an end time is specified, use that instead of steps to constrain output */
	if (NULL != cfg.end) {
		cfg.steps = 0;
	}
		
	/* interpret remaining command line arguments as paths to TLE files */
	for (int i = 0; i < argc; i++) {
		tles.push(ReadTleFromPath(argv[i]));
	}
		
	/* some attributes require an observer station to be defined; check if so */
	CheckAttributeObserver(has_observer);
	
	/* if we have not received any TLEs yet, attempt to read from stdin */
	if (tles.empty()) {
		tles.push(ReadTleFromStream(std::cin));
	}

	/* output a trace for each TLE */
	if (1 == tles.size()) {
		// special case where we treat --output as basename
		// and do not append any id numbers or prefix/suffix
		cfg.single = true;
	}
	
	while (!tles.empty()) {
		InitGroundTrace(tles.front(), now, cfg);
		tles.pop();
	}
	
	return EXIT_SUCCESS;
}
