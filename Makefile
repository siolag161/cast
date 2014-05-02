# The pre-processor and compiler options.
MY_CFLAGS =

# The linker options.
THIRD_PARTY = /home/pdt/dev/thirdparty
MY_LIBS = -L$(THIRD_PARTY)/boost_1_55_0/target/lib

INCL_DIRS = -I. -I$(THIRD_PARTY)/boost_1_55_0/target/include 

# The pre-processor options used by the cpp (man cpp for more).
CPPFLAGS = -Wall  -Wno-unused-local-typedefs
 
# The options used in linking as well as in any direct use of ld.
LDFLAGS = -lboost_log -lboost_program_options -lboost_system -lspl -lboost_serialization -lboost_thread -lboost_filesystem -lboost_regex

# The directories in which source files reside.
# If not specified, only the current directory will be searched.
SRCDIRS = ./ 

PROBT_LIB =  $(THIRD_PARTY)/probt-spl-2.3.0/lib

# The executable file name.
# If not specified, current directory name or `a.out' will be used.
PROGRAM = $(PROGDIR)/TESTFltm

## Implicit Section: change the following only when necessary.
##==========================================================================

# The source file types (headers excluded).
# .c indicates C source files, and others C++ ones.
SRCEXTS = .cpp
# The header file types.
HDREXTS = .hpp 

# The pre-processor and compiler options.
# Users can override those variables from the command line.
CXXFLAGS = -g -O3 -Wno-unused-function -std=c++11

# The C++ program compiler.
CXX    = g++

# The command used to delete file.
RM     = rm -f

ETAGS = etags
ETAGSFLAGS =

CTAGS = ctags
CTAGSFLAGS =

## Stable Section: usually no need to be changed. But you can add more.
##==========================================================================
SHELL   = /bin/sh
EMPTY   =
SPACE   = $(EMPTY) $(EMPTY)
ifeq ($(PROGRAM),)
	CUR_PATH_NAMES = $(subst /,$(SPACE),$(subst $(SPACE),_,$(CURDIR)))
	PROGRAM = $(word $(words $(CUR_PATH_NAMES)),$(CUR_PATH_NAMES))
ifeq ($(PROGRAM),)
	PROGRAM = a.out
endif
endif
ifeq ($(SRCDIRS),)
	SRCDIRS = .
endif

# Environment variable containing the path to the dynamically loaded libraries
ifeq ($(shell uname -s),Darwin)
# libraries for Mac OS X
	DLL_PATH_VAR := DYLD_LIBRARY_PATH
else
# libraries for Linux
	DLL_PATH_VAR := LD_LIBRARY_PATH
endif


SOURCES = $(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*,$(SRCEXTS))))
HEADERS = $(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*,$(HDREXTS))))
SRC_CXX = $(filter-out %.c,$(SOURCES))
OBJS    = $(addsuffix .o, $(basename $(SOURCES)))
DEPS    = $(OBJS:.o=.d)

## Define some useful variables.

COMPILE.cxx = $(CXX) $(INCL_DIRS) $(RCPPINCL)  $(MY_LIBS) $(MY_CFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c
LINK.cxx = $(CXX) $(MY_LIBS)  $(MY_CFLAGS) $(CXXFLAGS) $(CPPFLAGS) 

#.PHONY: all objs tags ctags clean distclean help show

# Delete the default suffixes
.SUFFIXES: # ~

PROGDIR =  bin

$(BEFORE): mkdir -p $(PROGDIR)

all: $(BEFORE) $(PROGRAM)

# Rules for generating object files (.o).
#----------------------------------------
objs:	$(OBJS)

%.o:	%.cpp
	$(COMPILE.cxx)  $< -o $@


# Rules for generating the executable.
#-------------------------------------
$(PROGRAM): $(OBJS)
	mkdir -p $(PROGDIR)
	$(DLL_PATH_VAR)=$(PROBT_LIB):  $(LINK.cxx) $(INCL_DIRS) $(OBJS)  -o $@ $(LDFLAGS)
	@echo Type ./$@ to execute the program.


BACKUP_SOURCES := $(shell find $(SOURCEDIR)  '*~')


clean:
	$(RM) $(OBJS) $(PROGRAM) $(PROGRAM).exe $(BACKUP_SOURCES)

distclean: clean
	$(RM) $(DEPS) TAGS

# Show help.
help:
	@echo 'Generic Makefile for C/C++ Programs (gcmakefile) version 0.5'
	@echo 'Copyright (C) 2007, 2008 whyglinux <whyglinux@hotmail.com>'
	@echo
	@echo 'Usage: make [TARGET]'
	@echo 'TARGETS:'
	@echo '  all       (=make) compile and link.'
	@echo '  NODEP=yes make without generating dependencies.'
	@echo '  objs      compile only (no linking).'
	@echo '  tags      create tag for Emacs editor.'
	@echo '  ctags     create ctags for VI editor.'
	@echo '  clean     clean objects and the executable file.'
	@echo '  distclean clean objects, the executable and dependencies.'
	@echo '  show      show variables (for debug use only).'
	@echo '  help      print this message.'
	@echo
	@echo 'Report bugs to <whyglinux AT gmail DOT com>.'

# Show variables (for debug use only.)
show:
	@echo 'PROGRAM     :' $(PROGRAM)
	@echo 'SRCDIRS     :' $(SRCDIRS)
	@echo 'HEADERS     :' $(HEADERS)
	@echo 'SOURCES     :' $(SOURCES)
	@echo 'SRC_CXX     :' $(SRC_CXX)
	@echo 'OBJS        :' $(OBJS)
	@echo 'DEPS        :' $(DEPS)
	@echo 'DEPEND      :' $(DEPEND)
	@echo 'COMPILE.cxx :' $(COMPILE.cxx)
	@echo 'link.cxx    :' $(LINK.cxx)

## End of the Makefile ##  Suggestions are welcome  ## All rights reserved ##
#############################################################################
