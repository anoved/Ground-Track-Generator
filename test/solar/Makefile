CPP = /usr/bin/g++
CFLAGS = -g -Wall -fPIC -Wextra
LIB_SGP4_DIR = ../../Libraries/sgp4/libsgp4
LIB_SGP4 = $(LIB_SGP4_DIR)/libsgp4.a
INCLUDES = -I $(LIB_SGP4_DIR)

# -lrt required on linux for libsgp4's use of clock_gettime
solarposition: solarposition.o $(LIB_SGP4)
	$(CPP) solarposition.o $(LIB_SGP4) -lrt -o $@

solarposition.o: solarposition.cpp
	$(CPP) $(CFLAGS) $(INCLUDES) -c solarposition.cpp
