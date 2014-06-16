CFLAGSS := -Wall -Wno-unknown-pragmas -Wunused-variable -Wsign-compare -Wno-format -mtune=native -msse3 -unroll-all-loops -fstrict-aliasing -ftree-vectorizer-verbose=2 -fpermissive -O3

SRCSS := DataUtility.cpp epnp.cpp MathUtility.cpp MultiviewGeometryUtility.cpp
OBJSS := $(addsuffix .o,$(basename ${SRCSS}))

all: abs

abs: AbsoluteRegistration.cpp ${OBJSS}
	g++ -o abs AbsoluteRegistration.cpp ${CFLAGSS} `pkg-config --libs --cflags opencv` ${OBJSS} 

%.o: %.cpp
	g++ -c ${CFLAGSS} $< -o $@

dist:
	bash makedistlinux

clean:
	-rm -f core ${OBJSS} abs *.o

depend:
epnp.o: epnp.h
MathUtility.o: MathUtility.h StructDefinition.h
DataUtility.o: DataUtility.h MathUtility.h StructDefinition.h
MultiviewGeometryUtility.o: MultiviewGeometryUtility.h MathUtility.h DataUtility.h epnp.h SBA/levmar.h SBA/sba.h
abs: DataUtility.h MultiviewGeometryUtility.h MathUtility.h sift.h
