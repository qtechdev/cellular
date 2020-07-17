SOURCES=$(wildcard src/*.cpp) $(wildcard src/*/*.cpp) $(wildcard src/*/*/*.cpp)
OBJECTS=$(patsubst src/%,build/%,${SOURCES:.cpp=.o})
DIRS=$(filter-out build/,$(sort $(dir ${OBJECTS})))

CXX=g++
LD_FLAGS=-ldl -lGL -lglfw -L./lib -lglad -lqfio -lqxdg
CXX_FLAGS=-std=c++17 -I./include

NAME=opengl
BINARY=out/${NAME}

ifdef DEBUG
CXX_FLAGS += -g -DDEBUG
endif
ifndef DEBUG
CXX_FLAGS += -O2
endif

all: dirs ${BINARY}

${BINARY}: ${OBJECTS}
	${CXX} $^ ${LD_FLAGS} -o $@

build/%.o: src/%.cpp
	${CXX} $< ${CXX_FLAGS} -c -o $@

.PHONY: dirs
dirs:
	mkdir -p ${DIRS}
	mkdir -p out/

.PHONY: clean
clean:
	-rm -r build/
	-rm -r out/
