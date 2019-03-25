#****************************************************************************
#
# Makefile
#
# This is a GNU make (gmake) makefile
#****************************************************************************

# DEBUG can be set to YES to include debugging info, or NO otherwise
DEBUG          ?= yes
#****************************************************************************

CC     ?= gcc
CXX    ?= g++
LD     ?= g++
AR     ?= ar
RANLIB ?= ranlib

DEBUG_CFLAGS     += -std=c++11 -ggdb -Wall -ffunction-sections -O0 -Wno-format \
-DDEBUG -DMYSQLPP_MYSQL_HEADERS_BURIED -DHAVE_SCHED_GET_PRIORITY_MAX -DLOG4CPP_FIX_ERROR_COLLISION -DLOG4CPP
RELEASE_CFLAGS   += -std=c++11 -ggdb -Wall -ffunction-sections -O3 -Wno-format \
-Wno-unknown-pragmas -Wno-format -DMYSQLPP_MYSQL_HEADERS_BURIED -DHAVE_SCHED_GET_PRIORITY_MAX -DLOG4CPP

LIBS		 :=

DEBUG_CXXFLAGS   += ${DEBUG_CFLAGS} 
RELEASE_CXXFLAGS += ${RELEASE_CFLAGS}

DEBUG_LDFLAGS    +=
RELEASE_LDFLAGS  +=

ifeq (yes, ${DEBUG})
   CFLAGS       += ${DEBUG_CFLAGS}
   CXXFLAGS     += ${DEBUG_CXXFLAGS}
   LDFLAGS      += ${DEBUG_LDFLAGS}
else
   CFLAGS       += ${RELEASE_CFLAGS}
   CXXFLAGS     += ${RELEASE_CXXFLAGS}
   LDFLAGS      += ${RELEASE_LDFLAGS}
endif

#****************************************************************************
# Include paths
#****************************************************************************

#INCS := -I/usr/include/g++-2 -I/usr/local/include
INCS := -I./includes


#****************************************************************************
# Makefile code common to all platforms
#****************************************************************************

CFLAGS   := ${CFLAGS} ${INCS}
CXXFLAGS := ${CXXFLAGS} ${INCS}

export CC CXX LD AR RANLIB CFLAGS CXXFLAGS LDFLAGS LIBS

OUTPUT_LIBS    := ./libs

.PHONY:all clean

all:
	mkdir -p ${OUTPUT_LIBS}
	$(MAKE) -C utils
	$(MAKE) -C pool

clean:
	rm -rf $(OUTPUT_INCLUDE) $(OUTPUT_LIBS)
	$(MAKE) -C utils clean
	$(MAKE) -C pool clean