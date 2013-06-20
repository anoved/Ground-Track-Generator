# Libraries

Ground Track Generator has two external dependencies: [Shapelib](http://shapelib.maptools.org/) and Dan Warner's [C++ SGP4 Satellite Library](http://www.danrw.com/sgp4-satellite.php).

## Shapelib

1. Go to <http://shapelib.maptools.org/> and download <http://download.osgeo.org/shapelib/shapelib-1.3.0b3.zip> from <http://download.osgeo.org/shapelib/>
2. Unzip the archive; rename the directory `shapelib` and put it in this directory.

## Dan Warner's C++ SGP4 Satellite Library

1. Go to <http://www.danrw.com/sgp4-satellite.php> and download <http://www.danrw.com/public/sgp4/archive/tip.zip>
2. Unzip the archive; rename the directory `sgp4` and put it in this directory.

Note that it may be necessary to [re-]run `aclocal`, `automake`, and `./configure` in `sgp4` before it can be built with `make`.
