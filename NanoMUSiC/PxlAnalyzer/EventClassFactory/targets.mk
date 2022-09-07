# Adding the generated dict as an dependency to the main executable
#~ $(PROGRAM): EventClassFactory/TEventClassDict.o

#~ clean:
#~ 	@rm -f $(PROGRAM) $(OBJECTS) $(DEPENDS)
#~ 	@rm -f $(LIBDIR)/TEventClassDict_rdict.pcm
#~ 	@rm -f $(LIBDIR)/libTEventClass.so
#~ 	@rm -f EventClassFactory/TEventClassDict.*

# Directory rules:
#~ $(BINDIR):
#~ 	@mkdir $(BINDIR)

#~ $(LIBDIR):
#~ 	@mkdir $(LIBDIR)
#~
#~ $(DEPDIR):
#~ 	@mkdir $(DEPDIR)

# Rules for shared libraries for interactive (py)ROOT:
#$(LIBDIR)/TEventClass.so: $(TEVENTCLASS) | $(LIBDIR)
#~ $(LIBDIR)/TEventClass.so: EventClassFactory/TEventClassDict.o EventClassFactory/TEventClass.o| $(LIBDIR)
#~ ifndef VERBOSE
#~ 	@echo "build EC so"
#~ 	g++ -shared -o lib/libTEventClass.so EventClassFactory/TEventClassDict.o EventClassFactory/TEventClass.o
#~ else
#~ 	$(BUILDSHARED)
#~ endif
#~
#~ EventClassFactory/TEventClassDict.cc: EventClassFactory/TEventClass.hh EventClassFactory/TEventClassLinkDef.h
#~ 	@echo "Generating TEventClass dictionary ..."
#~ 	@echo  -v2 -f $@ -c $(MUSIC_CFLAGS) $^
#~ 	@rootcint -v2 -f $@ -c $(MUSIC_CFLAGS) $^ # -v2: Display error and warning messages.
#~ 	@mv EventClassFactory/TEventClassDict_rdict.pcm $(LIBDIR)/TEventClassDict_rdict.pcm

#$(BINDIR)/ECCrossSectionRescaler: $(PROGSDIR)/ECCrossSectionRescaler.o $(TOOLS) Tools/AnyOption.o $(TEVENTCLASS) | $(BINDIR)
#ifndef VERBOSE
#	$(ECHO)
#	@$(BUILDBIN)
#else
#	$(BUILDBIN)
#endif

	# ~ @rootcint -v2 -f $@ -c -I$(MUSIC_BASE)/Main/ $^ # -v2: Display error and warning messages.
