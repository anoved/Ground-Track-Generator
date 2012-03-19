# Tools

CPP = /usr/bin/g++
AR = /usr/bin/ar
RM = /bin/rm

# Compiler Options

CFLAGS = -g -Wall -fPIC

# Objects

LIBOBJ = Eci.o Julian.o Observer.o OrbitalElements.o SGP4.o SolarPosition.o Timespan.o Tle.o Util.o

Eci.o: Eci.cpp Eci.h CoordGeodetic.h Vector.h Julian.h Globals.h
	$(CPP) $(CFLAGS) -c Eci.cpp

Julian.o: Julian.cpp Julian.h Globals.h Timespan.h
	$(CPP) $(CFLAGS) -c Julian.cpp

Observer.o: Observer.cpp Observer.h CoordGeodetic.h CoordTopographic.h Julian.h Eci.h
	$(CPP) $(CFLAGS) -c Observer.cpp

OrbitalElements.o: OrbitalElements.cpp OrbitalElements.h Tle.h
	$(CPP) $(CFLAGS) -c OrbitalElements.cpp

SGP4.o: SGP4.cpp SGP4.h Tle.h OrbitalElements.h Eci.h SatelliteException.h DecayedException.h
	$(CPP) $(CFLAGS) -c SGP4.cpp

SolarPosition.o: SolarPosition.cpp SolarPosition.h Julian.h Eci.h
	$(CPP) $(CFLAGS) -c SolarPosition.cpp

Timespan.o: Timespan.cpp Timespan.h
	$(CPP) $(CFLAGS) -c Timespan.cpp

Tle.o: Tle.cpp Tle.h Globals.h Util.h Julian.h TleException.h
	$(CPP) $(CFLAGS) -c Tle.cpp

Util.o: Util.cpp Util.h Globals.h
	$(CPP) $(CFLAGS) -c Util.cpp

# Library

lib: libsgp4.a

libsgp4.a: $(LIBOBJ)
	$(AR) r $@ $(LIBOBJ)

# Clean

clean:
	$(RM) -f $(LIBOBJ) libsgp4.a
