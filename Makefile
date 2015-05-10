TARGETS = asm stew
SOURCES = asm.cpp lineparser.cpp stew.cpp
OBJECTS = $(patsubst %.cpp,%.o,${SOURCES})

CXX = clang++
CXXFLAGS = -g -Wall -Werror -Wextra -Wno-unused-private-field -std=c++11

all: .depends ${TARGETS}

asm: asm.o lineparser.o
	${CXX} -o $@ $^

stew: stew.o lineparser.o
	${CXX} -o $@ $^

clean:
	rm ${OBJECTS} .depends

include .depends

.depends: Makefile ${SOURCES}
	${CXX} -MM ${CXXFLAGS} ${SOURCES} > $@

