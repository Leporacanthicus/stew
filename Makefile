TARGETS = asm
SOURCES = asm.cpp lineparser.cpp
OBJECTS = $(patsubst %.cpp,%.o,${SOURCES})

CXX = clang++
CXXFLAGS = -g -Wall -Werror -Wextra -std=c++11

all: .depends ${TARGETS}

asm: asm.o lineparser.o
	${CXX} -o $@ $^


clean:
	rm ${OBJECTS} .depends

include .depends

.depends: Makefile ${SOURCES}
	${CXX} -MM ${CXXFLAGS} ${SOURCES} > $@

