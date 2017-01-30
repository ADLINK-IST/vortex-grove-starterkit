# 
#                          OpenSplice DDS
# 
#    This software and documentation are Copyright 2006 to 2014 PrismTech
#    Limited and its licensees. All rights reserved. See file:
# 
#                      $OSPL_HOME/LICENSE
# 
#    for full copyright notice and license terms.
# 
#

IDLNAME    = VortexGrove
IDLFILE    = $(IDLNAME).idl
IDLDIR     = generated
EXES       = applicationMonitor lightPublisher rotaryPublisher LEDSubscriber

# Common variables
CC         = g++
CPPFLAGS   = -g -I$(OSPL_HOME)/include/sys -I$(OSPL_HOME)/include/dcps/C++/isocpp2 -I$(OSPL_HOME)/include/dcps/C++/SACPP -I$(IDLDIR) -Wall 
CPPFLAGS   += -I"/home/pi/Documents/PrismTech/GrovePi/Software/C/PTModified"
CXTRA      = -std=c++0x

SPPIDL     = idlpp -S -l isocpp2
SOURCES    = $(addsuffix .cpp, $(EXES))
GEN_CPP    = $(IDLNAME).cpp $(IDLNAME)SplDcps.cpp
GEN_H      = $(GEN_CPP:%.cpp=%.h) $(IDLNAME)_DCPS.hpp
GENERATED  = $(GEN_CPP) $(GEN_H)
BINARIES   = $(patsubst %.cpp, %.o, $(GEN_CPP))

LDLIBS     = -L$(OSPL_HOME)/lib -ldcpsisocpp2 -ldcpssacpp -lddskernel

vpath %.idl $(IDLDIR)

# Special targets
.PHONY: clean

#Dependencies
#gcc /home/pi/Documents/PrismTech/GrovePi/Software/C/grovepi.c -o /home/pi/Documents/PrismTech/GrovePi/Software/C/grovepiObject.o

all: $(EXES)

$(EXES): $(BINARIES) 

# Code generation
# This rule is explicit since the name of the
# targets depends on the name of the module in
# the IDL-file.

$(GENERATED): $(IDLFILE)
	$(SPPIDL) $(IDLFILE) -I$(OSPL_HOME)/include/dcps/C++/isocpp2 -I$(OSPL_HOME)/include/dcps/C++/SACPP

# Clean implicit rule because it is interesting
# to see the intermediate stages
%: %.cpp

%.o: %.cpp
	$(CC) -c $(CPPFLAGS) $(CXTRA) $(INCL) $^ -o $@

# Linking
%: %.o
	$(CC) $^ $(LDLIBS) -o $@

clean:
	@rm -rf $(EXES) $(BINARIES) $(GENERATED)
