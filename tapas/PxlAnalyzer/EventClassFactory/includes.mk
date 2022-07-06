# Set directories:
# Where to store generated libraries?
LIBDIR = lib
# Where to store the generate dependency files?
DEPDIR = dep
# Where to copy binaries?
BINDIR = bin
# Where are the .cc files for the executables?
PROGSDIR = Progs



#~ TARGETS	:= EventClassFactory/TEventClassDict.cc $(PROGRAM)
#~ TARGETS += $(LIBDIR)/TEventClass.so
#TARGETS += $(LIBDIR)/ConvolutionComputer_add.so

# Objects for EventClass(Factory):
#SRCS := $(wildcard EventClassFactory/*.cc)
#TEC  := $(patsubst %.cc,%.o,$(SRCS))
#TEVENTCLASS        := EventClassFactory/TEventClass.o EventClassFactory/TEventClassDict.o EventClassFactory/Resolutions.o $(TOOLS) $(PXL)
#TEVENTCLASSFACTORY := $(TEVENTCLASS) $(TEC)


#EventClassFactory/TEventClassDict.cc
#= EventClassFactory/TEventClassDict.cc
#~ MUSIC_CFLAGS := -isystem$(MUSIC_BASE)/Tools/ -isystem$(MUSIC_BASE)/Tools/PXL/ -isystem$(MUSIC_BASE)/Main
MUSIC_CFLAGS := -I$(MUSIC_BASE)/Tools/ -I$(MUSIC_BASE)/Tools/PXL/ -I$(MUSIC_BASE)/Main -I$(MUSIC_UTILS)/include
CFLAGS	+= $(MUSIC_CFLAGS)

MUSIC_LDFLAGS := -L$(MUSIC_UTILS)/lib -lTEventClass
LDFLAGS += $(MUSIC_LDFLAGS)

BUILDSHARED:= g++ -o $@ -shared $(LDFLAGS) -O $^

#ECHO = @echo -e "$(BOLDGREEN)Building$(NO_COLOR) $@ ..."

