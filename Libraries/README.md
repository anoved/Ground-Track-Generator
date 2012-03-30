# Libraries

Ground Track Generator has two external dependencies: Shapelib and Dan Warner's C++ SGP4 Satellite Library. At present, the Ground Track Generator build process expects the libraries to be installed in this directory and built using the provided Makefiles.

## Shapelib

1. Go to <http://shapelib.maptools.org/> and download <http://download.osgeo.org/shapelib/shapelib-1.3.0b3.zip> from <http://download.osgeo.org/shapelib/>
2. Unzip the archive; rename the directory `shapelib` and put it in this directory.
3. Move the `Makefile-shapelib` file provided here into `shapelib` and rename it `Makefile`.

## Dan Warner's C++ SGP4 Satellite Library

1. Go to <http://www.danrw.com/sgp4-satellite.php> and download <http://www.danrw.com/public/sgp4/archive/tip.zip>
2. Unzip the archive; rename the directory `sgp4` and put it in this directory.
3. MOve the `Makefile-sgp4` file provided here into `sgp4/libsgp4` and rename it `Makefile`.
