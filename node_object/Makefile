CFLAGSS := -Wall -Wno-unknown-pragmas -Wno-format -O3

SRCSS := tinyxml.cpp tinyxmlparser.cpp tinyxmlerror.cpp tinystr.cpp
OBJSS := $(addsuffix .o,$(basename ${SRCSS}))

all: init gui

init: init.cpp
	g++ -o init init.cpp

gui: gui.cpp ${OBJSS}
	gcc -o gui gui.cpp `pkg-config --libs --cflags gtk+-2.0 opencv` -lpthread ${OBJSS} 

gui2: gui2.cpp ${OBJSS}
	g++ -g -o gui2 gui2.cpp `pkg-config --libs --cflags gtk+-2.0 opencv` -lpthread ${OBJSS} 

%.o: %.cpp
	g++ -c ${CFLAGSS} $< -o $@

dist:
	bash makedistlinux

clean:
	-rm -f core ${OBJSS} gui init gui2 *.o

depend:
tinyxml.o: tinyxml.h tinystr.h
tinyxmlparser.o: tinyxml.h tinystr.h
tinyxmlerror.o: tinyxml.h tinystr.h
gui: readObject.h
gui2: readObject.h
