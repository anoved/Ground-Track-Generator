/* largely lifted from the runtest.cpp sample code distributed with SGP4++ */

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>

#include "gtgutil.h"
#include "gtgtle.h"

#include "Util.h"

void tokenize(const std::string& str, std::vector<std::string>& tokens)
{
    const std::string& delimiters = " ";

    /*
     * skip delimiters at beginning
     */
    std::string::size_type last_pos = str.find_first_not_of(delimiters, 0);

    /*
     * find first non-delimiter
     */
    std::string::size_type pos = str.find_first_of(delimiters, last_pos);

    while (std::string::npos != pos || std::string::npos != last_pos)
    {
        /*
         * add found token to vector
         */
        tokens.push_back(str.substr(last_pos, pos - last_pos));
        /*
         * skip delimiters
         */
        last_pos = str.find_first_not_of(delimiters, pos);
        /*
         * find next non-delimiter
         */
        pos = str.find_first_of(delimiters, last_pos);
    }
}


/* For loading a TLE from a stream */
Tle ReadTleFromStream(std::istream *stream)
{
    bool got_first_line = false;
    std::string line1;
    std::string line2;
    std::string parameters;
	bool got_tle = false;
	
    while (!stream->eof())
    {
        std::string line;
        std::getline(*stream, line);

        Util::Trim(line);

        /*
         * skip blank lines or lines starting with #
         */
        if (line.length() == 0 || line[0] == '#')
        {
            got_first_line = false;
            continue;
        }

        /*
         * find first line
         */
        if (!got_first_line)
        {
            try
            {
                if (line.length() >= Tle::GetLineLength())
                {
                    Tle::IsValidLine(line.substr(0, Tle::GetLineLength()), 1);
                    /*
                     * store line and now read in second line
                     */
                    got_first_line = true;
                    line1 = line;
                }
            }
            catch (TleException& e)
            {
            	Fail("%s\n%s\n", e.what(), line.c_str());
            }
        }
        else
        {
            /*
             * no second chances, second line should follow the first
             */
            got_first_line = false;
            /*
             * split line, first 69 is the second line of the tle
             * the rest is the test parameters, if there is any
             */
            line2 = line.substr(0, Tle::GetLineLength());
            double start = 0.0;
            double end = 1440.0;
            double inc = 120.0;
            if (line.length() > 69)
            {
                std::vector<std::string> tokens;
                parameters = line.substr(Tle::GetLineLength() + 1,
                        line.length() - Tle::GetLineLength());
                tokenize(parameters, tokens);
                if (tokens.size() >= 3)
                {
                    start = atof(tokens[0].c_str());
                    end = atof(tokens[1].c_str());
                    inc = atof(tokens[2].c_str());
                }
            }

            /*
             * following line must be the second line
             */
            try
            {
                if (line.length() >= Tle::GetLineLength())
                {
                    Tle::IsValidLine(line.substr(0, Tle::GetLineLength()), 2);
                    got_tle = true;
                    break;
                }
            }
            catch (TleException& e)
            {
            	Fail("%s\n%s\n", e.what(), line.c_str());
            }
        }
    }
	
	if (not got_tle) {
		Fail("No TLE could be found.\n");
	}
	
	return Tle("", line1, line2);
}

/* For loading a TLE from a file specified by a path */
Tle ReadTleFromPath(const char* infile)
{
	std::ifstream file(infile);
	if (!file.is_open()) {
		Fail("Could not open TLE file: %s\n", infile);
	}
	
	Tle tle = ReadTleFromStream(&file);

	file.close();	
	return tle;
}

/* For reading a Tle from a string such as an argument value */
Tle ReadTleFromBuffer(const char *buffer)
{
	std::istringstream sb(buffer);
	return ReadTleFromStream(&sb);
}
