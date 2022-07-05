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

ifndef CMSSW_RELEASE_BASE
   $(error Error: CMSSW libraries not found!)
endif

# Paths for CMSSW libraries:
CMSSW_DCAP_BASE  := $(shell cat $(CMSSW_RELEASE_BASE)/config/toolbox/$(SCRAM_ARCH)/tools/selected/dcap.xml | grep 'name="DCAP_BASE"' | sed -e 's/.*default="//' | sed -e 's/"\/>//')
CMSSW_BOOST_BASE := $(shell cat $(CMSSW_RELEASE_BASE)/config/toolbox/$(SCRAM_ARCH)/tools/selected/boost.xml | grep 'name="BOOST_BASE"' | sed -e 's/.*default="//' | sed -e 's/"\/>//')
CMSSW_GSL_BASE   := $(shell cat $(CMSSW_RELEASE_BASE)/config/toolbox/$(SCRAM_ARCH)/tools/selected/gsl.xml | grep 'name="GSL_BASE"' | sed -e 's/.*default="//' | sed -e 's/"\/>//')

# If you're using a patched CMSSW release, some of the libs are still in the base release, so you also have to look there.
CMSSW_RELEASE_BASE_NOPATCH := $(shell echo $(CMSSW_RELEASE_BASE) | sed -e 's/-patch//' -e 's/_patch.//')

CMSSW_LIB_PATHS := -L$(CMSSW_BASE)/lib/$(SCRAM_ARCH)
CMSSW_LIB_PATHS += -L$(CMSSW_RELEASE_BASE)/lib/$(SCRAM_ARCH)
CMSSW_LIB_PATHS += -L$(CMSSW_RELEASE_BASE_NOPATCH)/lib/$(SCRAM_ARCH)
CMSSW_LIB_PATHS += -L$(CMSSW_DCAP_BASE)/lib
CMSSW_LIB_PATHS += -L$(CMSSW_GSL_BASE)/lib
CMSSW_LIB_PATHS += -L$(CMSSW_BOOST_BASE)/lib

CMSSW_LIBS += -lCondFormatsJetMETObjects
CMSSW_LIBS += -lPhysicsToolsUtilities
CMSSW_LIBS += -ldcap
CMSSW_LIBS += -lgsl
CMSSW_LIBS += -lgslcblas
CMSSW_LIBS += -lboost_filesystem
CMSSW_LIBS += -lboost_date_time
CMSSW_LIBS += -lboost_serialization
CMSSW_LIBS += -lboost_program_options

# For the headers there are symlinks.
CMSSW_INC_PATHS := -isystem$(CMSSW_BASE)/src
CMSSW_INC_PATHS += -isystem$(CMSSW_RELEASE_BASE)/src
CMSSW_INC_PATHS += -isystem$(CMSSW_DCAP_BASE)/include
CMSSW_INC_PATHS += -isystem$(CMSSW_BOOST_BASE)/include
CMSSW_INC_PATHS += -isystem$(CMSSW_GSL_BASE)/include
CMSSW_INC_PATHS += -isystem$(LIBS3A)/include

EXTRA_CFLAGS  := -ffloat-store $(CMSSW_INC_PATHS) $(LHAPDF_INC_PATH)
EXTRA_LDFLAGS := $(CMSSW_LIB_PATHS) $(CMSSW_LIBS) $(LHAPDF_LIB_PATH) $(LHAPDF_LIB)

CC	:= g++
CFLAGS	:= -O3 -Wall -fPIC -fsignaling-nans -funsafe-math-optimizations -fno-rounding-math -fno-signaling-nans -fcx-limited-range -fno-associative-math -std=gnu++11

LD	:= g++
LDFLAGS	:= -O3fast -lz -fno-associative-math -std=gnu++11# -pg for gprof

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
ifndef PXLANA
	@echo "PXLANA not set, libs will not be copied"
	@echo $(PXLANA)
endif

# Directory rules:
$(BINDIR):
	@mkdir $(BINDIR)

$(LIBDIR):
	@mkdir $(LIBDIR)

$(ANALIBDIR):
	@mkdir $(ANALIBDIR)

$(DEPDIR):
	@mkdir $(DEPDIR)

$(ANALIBDIR)/libTEventClass.so: $(LIBDIR)/libTEventClass.so | $(ANALIBDIR)
	@echo install $(LIBDIR)/libTEventClass.so $(ANALIBDIR)
	install $(LIBDIR)/libTEventClass.so $(ANALIBDIR)/libTEventClass.so

$(ANALIBDIR)/TEventClassDict_rdict.pcm: src/TEventClassDict.cc | $(ANALIBDIR)
	@echo install $(LIBDIR)/TEventClassDict_rdict.pcm $(ANALIBDIR)
	install $(LIBDIR)/TEventClassDict_rdict.pcm $(ANALIBDIR)/TEventClassDict_rdict.pcm

# Rules for shared libraries for interactive (py)ROOT:
#$(LIBDIR)/TEventClass.so: $(TEVENTCLASS) | $(LIBDIR)
$(LIBDIR)/libTEventClass.so: src/TEventClassDict.o src/TEventClass.o | $(LIBDIR)
ifndef VERBOSE
	@echo "build EC so"
	$(LD) -shared $(LDFLAGS) -o $(LIBDIR)/libTEventClass.so src/TEventClassDict.o src/TEventClass.o
else
	$(LD) -o $@ -shared $(LDFLAGS) -O $^
endif

src/TEventClassDict.cc: include/TEventClass.hh include/TEventClassLinkDef.h | $(LIBDIR)
	@echo "Generating TEventClass dictionary ..."
	@# -v2: Display error and warning messages.
	rootcint -v2 -f $@ -c -I. $(MUSIC_CFLAGS) $^
	install src/TEventClassDict_rdict.pcm $(LIBDIR)/TEventClassDict_rdict.pcm

clean:
	@rm -f $(OBJECTS) $(DEPENDS)
	@rm -f $(LIBDIR)/TEventClassDict_rdict.pcm
	@rm -f $(LIBDIR)/libTEventClass.so
	@rm -f src/TEventClassDict.*


########################################
# rules

%.o : %.cc
	$(CC) -MD -MP $(CFLAGS) -c -o $@ $<

-include $(DEPENDS)
