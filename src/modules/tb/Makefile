FLAGS_COMMON = -Wall -Wno-reorder -Wno-variadic-macros -Wno-parentheses \
   -std=c++11 -fms-extensions -Wno-pedantic
FLAGS_STRICT = -Wno-long-long
FLAGS_WERROR =

PROJECT := tb_gpio

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

ESPMOD=../..
include $(ESPMOD)/../Makefile.vars
INCDIR=-I. -I$(SYSTEMC_INC_DIR) -I$(MODDIR) -I$(INTFDIR)
DEFINES=
CFLAGS=-g $(INCDIR) $(DEFINES)
CPPFLAGS=$(CFLAGS) $(SYSTEMC_CXXFLAGS) -std=c++11

## Project Source
SRCDIR=$(MODDIR)
SRC=$(SRCDIR)/gpio_base.cpp $(SRCDIR)/gpio_simple.cpp $(SRCDIR)/gpio_mix.cpp \
    $(SRCDIR)/gpio_mfmix.cpp $(SRCDIR)/gpio_mf.cpp $(SRCDIR)/gn_mixed.cpp
OBJ=$(foreach a,$(notdir $(SRC:%.cpp=%.o)),$(OBJDIR)/$a)

## TEST Files
TESTSRC=$(PROJECT).cpp sc_main.cpp
TESTOBJ=$(foreach a,$(notdir $(TESTSRC:%.cpp=%.o)),$(OBJDIR)/$a)

## Dependency Files
SRCS=$(SRC) $(TESTSRC)
DEPFILES=$(foreach a,$(notdir $(SRCS:%.cpp=%.d)),$(DEPDIR)/$a)

OBJS=$(OBJ) $(TESTOBJ)

VPATH=.:$(SRCDIR)

all: $(PROJECT).x

# The main simulation executable
$(PROJECT).x: $(OBJS) $(SYSTEMC_LIB_DIR)/libsystemc.a
	g++ -o$@ $(CPPFLAGS) $(OBJS) $(SYSTEMC_LDFLAGS) $(SYSTEMC_LIBS)

# And the remaining files use the generic options
$(OBJDIR)/%.o: %.cpp $(DEPDIR)/%.d | $(DEPDIR) $(OBJDIR)
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<

# We create the object and dependencies directories
$(OBJDIR):
	mkdir -p $@
$(DEPDIR):
	mkdir -p $@

$(DEPFILES):
include $(wildcard $(DEPFILES))

clean:
	rm -rf $(OBJDIR) $(DEPDIR)
	rm -f $(PROJECT).x
	rm -f *.vcd

%: RCS/%,v
