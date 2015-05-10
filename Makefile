TARGETS = asm stew
SOURCES = asm.cpp lineparser.cpp stew.cpp memory.cpp cpu.cpp command.cpp
OBJECTS = $(patsubst %.cpp,%.o,${SOURCES})

CXX = clang++
WARNINGS = -Wall -Werror -Wextra -Wno-unused-private-field \
	-Wno-unused-parameter
CXXFLAGS = -g -std=c++11 ${WARNINGS}

all: .depends ${TARGETS}

asm: asm.o lineparser.o
	${CXX} -o $@ $^

stew: stew.o lineparser.o cpu.o memory.o command.o
	${CXX} -o $@ $^

clean:
	rm ${OBJECTS} .depends

include .depends

.depends: Makefile ${SOURCES}
	${CXX} -MM ${CXXFLAGS} ${SOURCES} > $@

