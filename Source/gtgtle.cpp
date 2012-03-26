/*
 * gtgtle
 *
 * Parses two-line element sets of data used to compute satellite position and
 * velocity according to the SGP4 model. http://celestrak.com/columns/v04n03/
 */

#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>

#include "Util.h"
#include "Tle.h"

#include "gtgutil.h"

#include "gtgtle.h"

/*
 * Load two-line element sets from an input stream. Adds TLEs to tles queue.
 * Returns true if any TLEs were loaded, otherwise false.
 */
bool ReadTlesFromStream(std::istream& stream, std::queue<Tle>& tles)
{
	std::string  thisLine, prevLine;
	Tle *tle = NULL;
	bool got_at_least_one_tle = false;
	
	while (!stream.eof()) {
		
		// read a line from the stream
		getline(stream, thisLine);
		
		// At least two lines are needed for a TLE.
		if (prevLine.empty()) {
			prevLine = thisLine;
			continue;
		}
		
		// Try to read this and the previous lines as a TLE.
		try {
			tle = new Tle(prevLine, thisLine);
		} catch (TleException &e) {
			;
		}
		
		// Were we able to read these lines as a TLE?
		if (tle != NULL) {
			
			// Yes! Push a copy onto the queue.
			got_at_least_one_tle = true;
			tles.push(*tle);
			
			// Empty the temporary TLE bucket.
			delete tle;
			tle = NULL;
			
			// Clear this (and thus soon the previous) line to start fresh.
			thisLine.clear();
		}
		
		prevLine = thisLine;
	}
	
	return got_at_least_one_tle;
}

/*
 * Load two-line element sets from a file identified by path. Adds TLEs to tles
 * queue. Returns true if any TLEs were loaded, otherwise false.
 */
bool ReadTlesFromPath(const char *path, std::queue<Tle>& tles)
{
	std::ifstream file(path);
	if (!file.is_open()) {
		Fail("cannot open TLE file: %s\n", path);
	}
	bool got_some_tles = ReadTlesFromStream(file, tles);
	file.close();
	return got_some_tles;
}

/*
 * Load two-line element sets from a string buffer. Adds TLEs to tles queue.
 * Returns true if any TLEs were loaded, otherwise false.
 */
bool ReadTlesFromBuffer(const char *buffer, std::queue<Tle>& tles)
{
	std::istringstream bufferstream(buffer);
	return ReadTlesFromStream(bufferstream, tles);
}
