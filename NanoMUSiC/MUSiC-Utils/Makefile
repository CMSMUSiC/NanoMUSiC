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

#define all directories with source code
DIRS	:= $(MUSIC_UTILS)
DIRS	+= src
DIRS	+= include

# Set directories:
# Where to store generated libraries?
LIBDIR = lib
# ANALIBDIR
ANALIBDIR = $(PXLANA)/$(LIBDIR)
# Where to store the generate dependency files?
DEPDIR = dep
# Where to copy binaries?
BINDIR = bin

# define source files
SOURCES	:= $(wildcard *.cc)
SOURCES	+= $(foreach dir,$(DIRS),$(wildcard $(dir)/*.cc))

# define header, object and dependency files based on source files
HEADERS	:= $(SOURCES:.cc=.h)
HEADERS	+= $(SOURCES:.cc=.hh)
OBJECTS	:= $(SOURCES:.cc=.o)
DEPENDS	:= $(SOURCES:.cc=.d)


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

EXTRA_CFLAGS  := -ffloat-store $(CMSSW_INC_PATHS) $(LHAPDF_INC_PATH)
EXTRA_LDFLAGS := $(CMSSW_LIB_PATHS) $(CMSSW_LIBS) $(LHAPDF_LIB_PATH) $(LHAPDF_LIB)

CC	:= g++
CFLAGS	:= -O3 -Wall -fPIC -fsignaling-nans -funsafe-math-optimizations -fno-rounding-math -fno-signaling-nans -fcx-limited-range -fno-associative-math -std=gnu++17

LD	:= g++
LDFLAGS	:= -O3fast -lz -fno-associative-math -std=gnu++17# -pg for gprof

# Debug flags?
ifdef DEBUG
   CFLAGS = -O0 -Wall -fPIC -g -pg -fprofile-generate
   LDFLAGS = -O0 -g -pg -lz -fprofile-generate
endif

CFLAGS	+= -DMYPXLANA=$(MYPXLANA)/AnalysisComposer.hh
CFLAGS	+= -I. $(ROOT_CFLAGS) $(EXTRA_CFLAGS)

LDFLAGS  += $(ROOT_LDFLAGS) $(ROOT_GLIBS) $(SYSLIBS) -L. $(EXTRA_LDFLAGS)

MUSIC_CFLAGS := -I$(PXLANA)/Tools/ -Iinclude
#-I$(MUSIC_BASE)/Tools/PXL/ -I$(MUSIC_BASE)/Main

CFLAGS += $(MUSIC_CFLAGS)

########################################
# define colors for better overview

RED=$(shell tput setaf 1)
YELLOW=$(shell tput setaf 2)
GREEN=$(shell tput setaf 3)
BOLD=$(shell tput bold)
NORMAL=$(shell tput sgr0)


########################################
# define all targets

ifndef PXLANA
	TARGETS	:= $(LIBDIR)/libTEventClass.so
else
	TARGETS	:= $(PXLANA)/$(LIBDIR)/libTEventClass.so $(PXLANA)/$(LIBDIR)/TEventClassDict_rdict.pcm
endif

########################################
# targets

# target: dependency
#	@shellcommand
#	command

all: $(TARGETS)
	@echo "------------> Executing target: $@"
ifndef PXLANA
	@echo "PXLANA not set, libs will not be copied"
	@echo $(PXLANA)
endif

# Directory rules:
$(BINDIR):
	@echo "------------> Executing target: $@"
	@mkdir $(BINDIR)

$(LIBDIR):
	@echo "------------> Executing target: $@"
	@mkdir $(LIBDIR)

$(ANALIBDIR):
	@echo "------------> Executing target: $@"
	@mkdir $(ANALIBDIR)

$(DEPDIR):
	@echo "------------> Executing target: $@"
	@mkdir $(DEPDIR)

$(ANALIBDIR)/libTEventClass.so: $(LIBDIR)/libTEventClass.so | $(ANALIBDIR)
	@echo "------------> Executing target: $@"
	@echo install $(LIBDIR)/libTEventClass.so $(ANALIBDIR)
	install $(LIBDIR)/libTEventClass.so $(ANALIBDIR)/libTEventClass.so

$(ANALIBDIR)/TEventClassDict_rdict.pcm: src/TEventClassDict.cc | $(ANALIBDIR)
	@echo "------------> Executing target: $@"
	@echo install $(LIBDIR)/TEventClassDict_rdict.pcm $(ANALIBDIR)
	install $(LIBDIR)/TEventClassDict_rdict.pcm $(ANALIBDIR)/TEventClassDict_rdict.pcm

# Rules for shared libraries for interactive (py)ROOT:
#$(LIBDIR)/TEventClass.so: $(TEVENTCLASS) | $(LIBDIR)
$(LIBDIR)/libTEventClass.so: src/TEventClassDict.o src/TEventClass.o | $(LIBDIR)
	@echo "------------> Executing target: $@"
ifndef VERBOSE
	@echo "build EC so"
	$(LD) -shared $(LDFLAGS) -o $(LIBDIR)/libTEventClass.so src/TEventClassDict.o src/TEventClass.o
else
	$(LD) -o $@ -shared $(LDFLAGS) -O $^
endif

src/TEventClassDict.cc: include/TEventClass.hh include/TEventClassLinkDef.h | $(LIBDIR)
	@echo "------------> Executing target: $@"
	@echo "Generating TEventClass dictionary ..."
	@# -v2: Display error and warning messages.
	rootcint -v2 -f $@ -c -I. $(MUSIC_CFLAGS) $^
	install src/TEventClassDict_rdict.pcm $(LIBDIR)/TEventClassDict_rdict.pcm

clean:
	@echo "------------> Executing target: $@"
	@rm -f $(OBJECTS) $(DEPENDS)
	@rm -f $(LIBDIR)/TEventClassDict_rdict.pcm
	@rm -f $(LIBDIR)/libTEventClass.so
	@rm -f src/TEventClassDict.*


########################################
# rules

%.o : %.cc
	@echo "------------> Executing target: $@"
	$(CC) -MD -MP $(CFLAGS) -c -o $@ $<

-include $(DEPENDS)
