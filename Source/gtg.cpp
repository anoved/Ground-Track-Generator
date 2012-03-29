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
	std::queue<Tle> tles;
	GTGConfiguration cfg;
	Timespan interval;
	
	/* All attributes turned off by default */
	FlagAllAttributes(false);
	
	/* Initialize default configuration */
	cfg.start = NULL; /* NULL start implies epoch start time */
	cfg.end = NULL;
	cfg.forceend = 0;
	cfg.unit = 'm';
	cfg.interval = 1.0;
	cfg.steps = 100;
	cfg.basepath = NULL;
	cfg.features = point;
	cfg.split = 0;
	cfg.has_observer = false;
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
			{"forceend", no_argument, &cfg.forceend, 1},
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
				
				cfg.has_observer = true;
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
						
			case 'l':
				/* Interval length */

				if (2 != sscanf(optarg, "%lf%c", &cfg.interval, &cfg.unit)) {
					Fail("cannot parse interval: %s\n", optarg);
				}
				
				if ((cfg.unit != 's') && (cfg.unit != 'm') && (cfg.unit != 'h') && (cfg.unit != 'd')) {
					Fail("invalid interval unit: %c (should be s, m, h, or d)\n", cfg.unit);
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
				ReadTlesFromBuffer(optarg, tles);
				break;
			
			case 'i':
				/* Input file */
				/* Argument format: path to file to read for TLE, ala runtest */\
				ReadTlesFromPath(optarg, tles);
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
	
	/* Compute step interval in whatever units were specified */
	switch (cfg.unit) {
		case 's': interval.AddSeconds(cfg.interval); break;
		case 'm': interval.AddSeconds(cfg.interval * 60.0); break;
		case 'h': interval.AddSeconds(cfg.interval * 60.0 * 60.0); break;
		case 'd': interval.AddSeconds(cfg.interval * 60.0 * 60.0 * 24.0); break;
	}
	Note("Step interval: %.9lf seconds\n", interval.GetTotalSeconds());
	
	/* interpret remaining command line arguments as paths to TLE files */
	for (int i = 0; i < argc; i++) {
		ReadTlesFromPath(argv[i], tles);
	}

	/* if we have not received any TLEs yet, attempt to read from stdin */
	if (tles.empty()) {
		ReadTlesFromStream(std::cin, tles);
	}
		
	if (1 == tles.size()) {
		// special case where we treat --output as basename
		// and do not append any id numbers or prefix/suffix
		cfg.single = true;
	}
	
	/* output a trace for each TLE */
	while (!tles.empty()) {
		InitGroundTrace(tles.front(), now, cfg, interval);
		tles.pop();
	}
		
	return EXIT_SUCCESS;
}
