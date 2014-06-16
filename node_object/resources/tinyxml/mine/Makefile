CFLAGSS := -Wall -Wno-unknown-pragmas -Wno-format -O3

SRCSS := tinyxml.cpp tinyxmlparser.cpp tinyxmlerror.cpp tinystr.cpp readObject.cpp
OBJSS := $(addsuffix .o,$(basename ${SRCSS}))

all: readObject

readObject: ${OBJSS}
	g++ -o $@ ${OBJSS} 

%.o : %.cpp
	g++ -c ${CFLAGSS} $< -o $@

dist:
	bash makedistlinux

clean:
	-rm -f core ${OBJSS} readObject

depend:
tinyxml.o: tinyxml.h tinystr.h
tinyxmlparser.o: tinyxml.h tinystr.h
tinyxmlerror.o: tinyxml.h tinystr.h
readObject.o: tinyxml.h tinystr.h
