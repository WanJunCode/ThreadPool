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

MYLIBS := -lpool -lutils

LIBS := -L../common/libs/ -L../module/libs/ -pthread -llog4cpp
	# -luuid -lmysqlpp -lmysqlclient -lpthread -levent -lmosquittopp -ljsoncpp -llog4cpp


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
INCS := -I$(shell pwd)/../common/ -I$(shell pwd)/../module/


#****************************************************************************
# Makefile code common to all platforms
#****************************************************************************

CFLAGS   := ${CFLAGS} ${INCS}
CXXFLAGS := ${CXXFLAGS} ${INCS}

#****************************************************************************
# Targets of the build
#****************************************************************************

BIN := bin
OUTPUT := ../$(BIN)/wj-client

all: ${OUTPUT}

#****************************************************************************
# Source files
#****************************************************************************
#
C_SRCS=$(shell find -iname "*.c")
#
CXX_SRCS=$(shell find -iname "*.cpp")

OBJS=$(subst .c,.o,$(C_SRCS))
OBJS+=$(subst .cpp,.o,$(CXX_SRCS))

#****************************************************************************
# Output
#****************************************************************************

${OUTPUT}:$(OBJS)
	$(CXX) -o $@ -Wl,-gc-sections -Wl,--start-group $(OBJS) $(LDFLAGS) $(LIBS)

#****************************************************************************
# common rules
#****************************************************************************

clean:
	-rm -f core ${OBJS} ${OUTPUT}
	-rm -f *.a *.o *.so* ../core
