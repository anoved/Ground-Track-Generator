# Tools

CPP = /usr/bin/g++
RM = /bin/rm

# Setup

CFLAGS = -g -Wall -fPIC

LIB_SGP4_DIR  = Libraries/sgp4/libsgp4
LIB_SHP_DIR   = Libraries/shapelib

INCLUDES = -I $(LIB_SGP4_DIR) -I $(LIB_SHP_DIR)

LIB_SGP4 = $(LIB_SGP4_DIR)/libsgp4.a
LIB_SHP  = $(LIB_SHP_DIR)/libshp.a

.PHONY: libs libsgp4 libshp $(LIB_SGP4_DIR) $(LIB_SHP_DIR) clean-libs clean-sgp4 clean-shp clean clean-all

# Ground Track Generator

gtg: $(LIB_SGP4) $(LIB_SHP) gtgshp.o gtgtrace.o gtgutil.o gtgtle.o gtg.o
	$(CPP) $(LIB_SGP4) $(LIB_SHP) gtgshp.o gtgtrace.o gtgutil.o gtgtle.o gtg.o -o gtg

gtg.o: gtg.cpp gtg.h gtgutil.h gtgtrace.h
	$(CPP) $(CFLAGS) $(INCLUDES) -c gtg.cpp

gtgtle.o: gtgtle.cpp gtgtle.h gtgutil.h
	$(CPP) $(CFLAGS) $(INCLUDES) -c gtgtle.cpp

gtgutil.o: gtgutil.cpp gtgutil.h
	$(CPP) $(CFLAGS) $(INCLUDES) -c gtgutil.cpp

gtgtrace.o: gtgtrace.cpp gtgtrace.h gtgutil.h gtgtle.h gtg.h gtgshp.h
	$(CPP) $(CFLAGS) $(INCLUDES) -c gtgtrace.cpp

gtgshp.o: gtgshp.cpp gtgshp.h gtg.h gtgutil.h
	$(CPP) $(CFLAGS) $(INCLUDES) -c gtgshp.cpp
	
# Prerequisite Libraries

libs: libsgp4 libshp

$(LIB_SGP4): libsgp4

$(LIB_SHP): libshp

libsgp4: $(LIB_SGP4_DIR)

libshp: $(LIB_SHP_DIR)

$(LIB_SGP4_DIR):
	@echo "# Making libsgp4..."
	@$(MAKE) --directory=$(LIB_SGP4_DIR) lib

$(LIB_SHP_DIR):
	@echo "# Making libshp..."
	@$(MAKE) --directory=$(LIB_SHP_DIR) lib

# Clean

clean:
	$(RM) -f *.o testapp

clean-all: clean clean-libs

clean-libs: clean-sgp4 clean-shp

clean-sgp4:
	$(MAKE) --directory=$(LIB_SGP4_DIR) clean

clean-shp:
	$(MAKE) --directory=$(LIB_SHP_DIR) clean
