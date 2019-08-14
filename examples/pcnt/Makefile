################################################################################
# Makefile -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
################################################################################
# Description:
#   Makefile for a ESPMOD Testbench
################################################################################
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
################################################################################

SYSTEMC_HOME=/opt/systemc/systemc-2.3.3
TARGET_ARCH=linux64
ARCH_SUFFIX=64-linux64
FLAGS_COMMON = -Wall -Wno-reorder -Wno-variadic-macros -Wno-parentheses \
   -std=c++11 -fms-extensions -Wno-pedantic
FLAGS_STRICT = -Wno-long-long
FLAGS_WERROR =

PROJECT := pcnt

SYSTEMC_INC_DIR=$(SYSTEMC_HOME)/include
SYSTEMC_LIB_DIR=$(SYSTEMC_HOME)/lib$(ARCH_SUFFIX)
SYSTEMC_CXXFLAGS=$(FLAGS_COMMON) $(FLAGS_STRICT) $(FLAGS_WERROR) -pthread
LDFLAG_RPATH=-Wl,-rpath=
SYSTEMC_LDFLAGS=-L $(SYSTEMC_LIB_DIR) $(LDFLAG_RPATH)$(SYSTEMC_LIB_DIR) \
   -lpthread
SYSTEMC_LIBS=-lsystemc -lm

OBJDIR := objs
DEPDIR := deps
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$(notdir $*.d)

COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) -c
COMPILE.cpp = $(CXX) $(DEPFLAGS) $(CPPFLAGS) -c

ARDUINOHOME=/opt/arduino/arduino-1.8.9
ARDUINOLIB=$(ARDUINOHOME)/hardware/espressif/esp32/libraries
ESPIDF=$(ARDUINOHOME)/hardware/espressif/esp32/tools/sdk/include
ESPLIB=$(HOME)/work/espmod/libespmod.a $(HOME)/work/espmod/libardidfmod.a
ESPLIBINC=-Wl,--start-group $(ESPLIB) -Wl,--end-group
ESPMOD=$(HOME)/work/espmod/src
include $(ESPMOD)/../Makefile.vars
INCDIR=-I. -I$(SRCDIR) $(INCLUDES)
	 #-I$(ESPIDF)/spi_flash 
DEFINES=-DDEBUG -DESP32 -DSYSCMOD
CFLAGS=-g $(INCDIR) $(DEFINES)
CPPFLAGS=$(CFLAGS) $(SYSTEMC_CXXFLAGS) -std=c++11

## Project Source
SRCDIR=.
SRC=
OBJ=$(foreach a,$(notdir $(SRC:%.cpp=%.o)),$(OBJDIR)/$a)

## TEST Files
TESTSRC=$(PROJECT).cpp $(PROJECT)test.cpp sc_main.cpp
TESTOBJ=$(foreach a,$(notdir $(TESTSRC:%.cpp=%.o)),$(OBJDIR)/$a)

## Dependency and Objects
SRCS=$(SRC) $(TESTSRC)
DEPFILES=$(foreach a,$(notdir $(SRCS:%.cpp=%.d)),$(DEPDIR)/$a)
OBJS=$(OBJ) $(TESTOBJ)

VPATH=.:$(SRCDIR)

all: $(PROJECT).x

# The main simulation executable
$(PROJECT).x: $(OBJS) $(SYSTEMC_LIB_DIR)/libsystemc.a $(ESPLIB)
	cd $(ESPMOD)/.. && make
	g++ -o$@ $(CPPFLAGS) $(OBJS) $(ESPLIBINC) $(SYSTEMC_LDFLAGS) \
           $(SYSTEMC_LIBS)

# And the remaining files use the generic options
$(OBJDIR)/%.o: %.cpp $(DEPDIR)/%.d | $(DEPDIR) $(OBJDIR)
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<

# The main cpp file needs to be generated. It is a wrapper for the ino file.
$(PROJECT).cpp:
	echo "#include \"Arduino.h\"" > $(PROJECT).cpp
	echo "#include \"$(SRCDIR)/$(PROJECT).ino\"" >> $(PROJECT).cpp

# We create the object and dependencies directories
$(OBJDIR):
	mkdir -p $@
$(DEPDIR):
	mkdir -p $@

$(DEPFILES):
include $(wildcard $(DEPFILES))

clean:
	rm -rf $(OBJDIR) $(DEPDIR)
	rm -f $(PROJECT).x testbar $(PROJECT).cpp
	rm -f *.vcd
