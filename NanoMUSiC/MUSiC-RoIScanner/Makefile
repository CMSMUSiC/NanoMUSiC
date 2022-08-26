### definition of the compiler options ###
#       -I location of directory containing include files
#       -L location of directory containing libraries
#       -lname include the library from -L location called libname.a
#          -lg2c is the library containing info on converting fortran to C
#          -lf   is the library containing the intrinsic for HPUX only.
#       -shared make a shared library as output
#       -fPIC produce position independent code
#        necessary on some platforms (including HPUX) for -shared
#       -fpic ^^ same(?)
#       -O optimizes
#       -g produces output for the debugger
#       -pg produces output for gprof profiler
#       note: if you want to see all warnings and ensure ansi standard
#             compatibility, use:
#             -pipe -ansi -pedantic -fnonnull-objects \
#             -W -Wall -Wwrite-strings -Wpointer-arith -Wnested-externs \
#             -Woverloaded-virtual -Wbad-function-cast -fnonnull-objects
#       The proper order for cernlib libraries is:
#       -lpawlib -lgraflib -lgrafX11 -lmathlib -lkernlib -lpacklib -ljetset74
#
# makefile syntax:
#        for target thedir/target.suf from source anotherdir/source.suf2
#        ${*D}  = thedir
#        ${*F}  = target
#        $^     = name of all prerequisites
#        $+     = like $^ with duplicated prerequisites if mentioned more than once
#        $*     = thedir/target
#        $@     = thedir/target.suf
#        $<     = anotherdir/source.suf2
#

########################################
# directories
# by default, the validator is compiled.
# usually one will change this to their personal analysis.

#~ ifndef MYPXLANA
	#~ MYPXLANA:=Validator
#~ endif
SHELL:=/bin/bash -O globstar
#define all directories with source code
#~ DIRS	:= $(MYPXLANA)
DIRS	+= include
DIRS	+= include/rapidjson
DIRS	+= src

# define source files
SOURCES	:= $(wildcard *.cc)
SOURCES	+= $(foreach dir,$(DIRS),$(wildcard $(dir)/*.cc))

# define header, object and dependency files based on source files
HEADERS	:= $(SOURCES:.cc=.h)
OBJECTS	:= $(SOURCES:.cc=.o)
SHARED_OBJECTS := $(SOURCES:.cc=.so)
DEPENDS	:= $(SOURCES:.cc=.d)

LOOKUPTABLE := bin/lookuptable.bin

########################################
# compiler and flags

# ROOT libraries:
ROOT_CFLAGS  := $(shell root-config --cflags)
ROOT_LDFLAGS := $(shell root-config --ldflags)
ROOT_GLIBS   := $(shell root-config --libs)

# LCG Base dir
LCG_BASE := /cvmfs/sft.cern.ch/lcg/views/LCG_102/x86_64-centos7-gcc11-opt

LHAPDF_LIB_PATH := -L$(LCG_BASE)/lib
LHAPDF_INC_PATH := -I$(-I)/include/LHAPDF
LHAPDF_LIB := -lLHAPDF

LCG_INC_PATHS := -I$(LCG_BASE)/include
BOOST_INC_PATHS += -I$(LCG_BASE)/include/boost
GSL_INC_PATHS += -I$(LCG_BASE)/include/gsl
LIBS3A_INC_PATHS := -I$(LIBS3A)/include

LCG_LIB_PATHS := -L$(LCG_BASE)/lib
LCG_LIB_PATHS += -L$(LCG_BASE)/lib64

LCG_LIBS := -ldcap
LCG_LIBS += -lgsl
LCG_LIBS += -lgslcblas
LCG_LIBS += -lboost_system
LCG_LIBS += -lboost_thread
LCG_LIBS += -lboost_filesystem
LCG_LIBS += -lboost_date_time
LCG_LIBS += -lboost_serialization
LCG_LIBS += -lboost_program_options

CC	:= g++
CFLAGS	:= -Wall -Wextra -std=c++17 -g

LD	:= g++
LDFLAGS	:= -std=c++17 -g

# Debug flags?
ifdef DEBUG
    CFLAGS += -Og -pg
    LDFLAGS += -Og -pg
else
    CFLAGS += -O3 -ffast-math
    LDFLAGS += -O3 -fwhole-program
endif

#~ CFLAGS	+= -DMYTEventClass=$(PXLANA)/$(MYPXLANA)/TEventClass.hh
#~ CFLAGS	+= -I$(MUSIC_BASE)/$(MYPXLANA)/ -I$(MUSIC_BASE)/Tools/ -I$(MUSIC_BASE)/Main/

CFLAGS  += -Iinclude/rapidjson
CFLAGS  += -Iinclude
CFLAGS  += $(LCG_INC_PATHS)
CFLAGS  += $(BOOST_INC_PATHS)
CFLAGS  += $(GSL_INC_PATHS)
CFLAGS  += $(LIBS3A_INC_PATHS)
CFLAGS  += $(ROOT_CFLAGS)
# CFLAGS  += -lboost_filesystem
# CFLAGS  += -lboost_program_options

LDFLAGS  += $(LCG_LIB_PATHS)
LDFLAGS  += $(LCG_LIBS)
LDFLAGS  += $(ROOT_LDFLAGS)
LDFLAGS  += $(ROOT_GLIBS)
LDFLAGS  += -lboost_filesystem
LDFLAGS  += -lboost_program_options
LDFLAGS  += -Iinclude/rapidjson
LDFLAGS  += -Iinclude

########################################
# define colors for better overview

RED=$(shell tput setaf 1)
YELLOW=$(shell tput setaf 2)
GREEN=$(shell tput setaf 3)
BOLD=$(shell tput bold)
NC=$(shell tput sgr0)


########################################
# define all target executables

EXECUTABLES	:= src/scanClass src/pCalc src/createLookupTable

########################################
# targets

# target: dependency
#	@shellcommand
#	command

.PHONY: clean

all: $(EXECUTABLES) src/ConvolutionComputer.so src/ConvolutionLookup.so

clean:
	rm -f $(OBJECTS) $(SHARED_OBJECTS) $(DEPENDS)

src/ConvolutionLookup.so: src/ConvolutionComputer.o src/ConvolutionLookup.o
src/pCalc: src/ConvolutionComputer.o src/ConvolutionLookup.o src/pCalc.o bin/lookuptable.bin
src/scanClass: src/ConvolutionComputer.o src/ECScanner.o src/Dicer.o src/MCBin.o src/ScanResult.o src/scanClass.o src/util.o src/ConvolutionLookup.o bin/lookuptable.bin
src/createLookupTable: src/ConvolutionComputer.o src/ConvolutionLookup.o src/createLookupTable.o

$(EXECUTABLES):
	@echo "Linking $@ ..."
	$(LD) $(LDFLAGS) $(filter %.o,$^) -o $@
	@echo "$@ done"
	@mkdir -p bin
	@install $@ bin/

$(LOOKUPTABLE): src/createLookupTable
	@echo "$(RED)The lookup-table seems to be out of-date!"
	@echo "Don't forget to regenerate it using 'make lut' (this might take up to several hours).$(NC)"

lut:
	@echo "Generating lookup table $@..."
	src/createLookupTable $(LOOKUPTABLE)

########################################
# rules

%.o : %.cc
	@echo "Compiling..."
	$(CC) -c -MD -MP -fPIC $(CFLAGS) -o $@ $<

%.so: %.o
	@echo "Linking shared object library $@..."
	$(CC) -lc -shared $(CFLAGS) -o $@ $^ -Wl,--whole-archive $(LDFLAGS) -Wl,--no-whole-archive

-include $(DEPENDS)
