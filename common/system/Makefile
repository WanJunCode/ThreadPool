.PHONY:all clean
#****************************************************************************
# Targets of the build
#****************************************************************************

INSTALL_INCLUDE_FILES := $(shell find -iname "*.h")
OUTPUT := libsystem.a

OUTPUT_INCLUDE := ../includes/system
OUTPUT_LIBS    := ../libs

all: ${OUTPUT}
	cp ${OUTPUT} ${OUTPUT_LIBS}

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

CXXFLAGS += -DHAVE_SCHED_GET_PRIORITY_MAX

${OUTPUT}: ${OBJS}
	${AR} rc $@ ${OBJS} ${LIBS} ${EXTRA_LIBS}
#	${LD} -shared -fPIC -o $@ ${LDFLAGS} ${OBJS} ${LIBS} ${EXTRA_LIBS}

#****************************************************************************
# common rules
#****************************************************************************

clean:
	-rm -f core ${OBJS} ${OUTPUT}
	-rm -f ${OUTPUT_LIBS}/${OUTPUT}
