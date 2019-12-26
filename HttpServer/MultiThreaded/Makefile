
# You may use and submit this makefile.
SOURCES=httpserver.cpp
INCLUDES=$(wildcard *.h)

TARGET=httpserver

CXXFLAGS=-std=gnu++11 -Wall -Wextra -Wpedantic -Wshadow -g -Og -pthreads

LDFLAGS= -pthreads


_submit_CXXFLAGS=-std=gnu++11 -Wall -Wextra -Wpedantic -Wshadow -g -O2 -pthreads




OBJECTS=$(SOURCES:.cpp=.o)


DEPS=$(SOURCES:.cpp=.d)

# We're using clang as our C++ compiler.
CXX=clang++


all: $(TARGET)

clean:
	-rm $(DEPS) $(OBJECTS)

spotless: clean
	-rm $(TARGET)


format:
	clang-format -i $(SOURCES) $(INCLUDES)

$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJECTS)


%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -MD -o $@ $<

-include $(DEPS)


.PHONY: all clean format spotless
