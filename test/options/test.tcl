#!/usr/bin/env tclsh

package require Tcl 8.5

set gtg ../../gtg


# TLE input methods - single TLE

#exec $gtg iss.tle
#exec $gtg < iss.tle
#exec $gtg --input iss.tle
#exec $gtg --tle "ISS (ZARYA)             \n1 25544U 98067A   12085.20387446  .00017924  00000-0  22587-3 0  2432\n2 25544  51.6423 183.0335 0016084 193.8608 250.6436 15.59550052764921"

# in all the above cases, output goes to a shapefile in the current direct named for the TLE NORAD number (25544 in the case of ISS)

# --output with a single source tle specifies entire base path
#exec $gtg --output foo iss.tle

# --prefix with a single source tle uses prefix+tlenumber as basename (foo25544)
#exec $gtg --prefix foo iss.tle

# 25544bar
#exec $gtg --suffix bar iss.tle

# foo25544bar
#exec $gtg --prefix foo --suffix bar iss.tle

# subdir/ exists, but --output is only basename, so output is subdir.shp/shx/dbf
#exec $gtg --output subdir iss.tle

# --prefix and --suffix are ignored if --output is given with a single TLE
#exec $gtg --output subdir --prefix foo iss.tle

# stations contains multiple tles; each written to cwd, using NORAD number as basename
# 25544, 37820, 37877, 38036, 38073, 38096
#exec $gtg stations.tle

# fails; with multiple tle, --output is expected to be a directory
#exec $gtg --output foo stations.tle

# subdir exists; output is written to that dir with NORAD number as basename
#exec $gtg --output subdir stations.tle

# outputs all with foo prefix to cwd
#exec $gtg --prefix foo stations.tle

# written to subdir/ with foo prefix
#exec $gtg --output subdir --prefix foo stations.tle

#exec $gtg --output subdir --suffix bar stations.tle

#exec $gtg --output subdir --prefix foo --suffix bar stations.tle

# a wgs-72 prj file should accompany each output shapefile
#exec $gtg --output subdir --prj stations.tle

# shape 46 is split over dateline
#exec $gtg --input iss.tle --features line
#exec $gtg --input iss.tle --features line --split --suffix split
#shpdump 25544.shp > 25544.dump
#shpdump 25544split.shp > 25544split.dump
#diff 25544.dump 25544split.dump

# forceend outputs a record exactly at --end.
# 
#exec $gtg --input iss.tle --start epoch --end epoch+10m --interval 1m --attributes mfe
#exec $gtg --input iss.tle --start epoch --end epoch+10m --interval 1m --forceend --suffix forced --attributes mfe
#exec $gtg --input iss.tle --prefix b --start epoch --end epoch+9.5m --interval 1m --attributes mfe
#exec $gtg --input iss.tle --prefix b --start epoch --end epoch+9.5m --interval 1m --forceend --suffix forced --attributes mfe
