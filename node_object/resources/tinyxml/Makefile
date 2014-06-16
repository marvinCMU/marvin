CC     := gcc
CXX    := g++
LD     := g++
AR     := ar rc
RANLIB := ranlib

   CFLAGS       := -Wall -Wno-unknown-pragmas -Wno-format -O3


#****************************************************************************
# Include paths
#****************************************************************************

#INCS := -I/usr/include/g++-2 -I/usr/local/include
INCS :=

CFLAGS   := -Wall -Wno-unknown-pragmas -Wno-format -O3

SRCS := tinyxml.cpp tinyxmlparser.cpp xmltest.cpp tinyxmlerror.cpp tinystr.cpp

OBJS := $(addsuffix .o,$(basename ${SRCS}))


all: xmltest



xmltest: ${OBJS}
	g++ -o $@ ${OBJS} 

%.o : %.cpp
	g++ -c ${CFLAGS} $< -o $@

dist:
	bash makedistlinux

clean:
	-rm -f core ${OBJS} xmltest

depend:
	#makedepend ${INCS} ${SRCS}

tinyxml.o: tinyxml.h tinystr.h
tinyxmlparser.o: tinyxml.h tinystr.h
xmltest.o: tinyxml.h tinystr.h
tinyxmlerror.o: tinyxml.h tinystr.h
