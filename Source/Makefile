# GTG/Source Makefile

# Tools

CPP = /usr/bin/g++
AR = /usr/bin/ar
RM = /bin/rm

# Settings

CFLAGS = -g -Wall -fPIC -Wextra
LIB_SGP4_DIR  = ../Libraries/sgp4/libsgp4
LIB_SHP_DIR   = ../Libraries/shapelib
INCLUDES = -I $(LIB_SGP4_DIR) -I $(LIB_SHP_DIR)
OBJS = gtg.o gtgtle.o gtgutil.o gtgtrace.o gtgshp.o gtgattr.o

.PHONY: default lib clean

# Objects

default: $(OBJS)

gtg.o: gtg.cpp gtg.h gtgutil.h gtgtrace.h
	$(CPP) $(CFLAGS) $(INCLUDES) -c gtg.cpp

gtgtle.o: gtgtle.cpp gtgtle.h gtgutil.h
	$(CPP) $(CFLAGS) $(INCLUDES) -c gtgtle.cpp

gtgutil.o: gtgutil.cpp gtgutil.h
	$(CPP) $(CFLAGS) $(INCLUDES) -c gtgutil.cpp

gtgtrace.o: gtgtrace.cpp gtgtrace.h gtgutil.h gtgtle.h gtg.h gtgshp.h gtgattr.h
	$(CPP) $(CFLAGS) $(INCLUDES) -c gtgtrace.cpp

gtgshp.o: gtgshp.cpp gtgshp.h gtg.h gtgutil.h
	$(CPP) $(CFLAGS) $(INCLUDES) -c gtgshp.cpp

gtgattr.o: gtgattr.cpp gtgattr.h gtg.h gtgutil.h
	$(CPP) $(CFLAGS) $(INCLUDES) -c gtgattr.cpp


# Library

lib: libgtg.a

libgtg.a: $(OBJS)
	$(AR) r $@ $(OBJS)

# Clean

clean:
	$(RM) -f $(OBJS) libgtg.a
