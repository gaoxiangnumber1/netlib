NETLIB_DIRECTORY = $(HOME)/netlib
NETLIB_INCLUDE = $(NETLIB_DIRECTORY)/include
NETLIB_LIBRARY = $(NETLIB_DIRECTORY)/lib

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wconversion -Werror -Wextra -Wno-unused-parameter \
						-Wold-style-cast -Woverloaded-virtual -Wpointer-arith -Wshadow \
						-Wwrite-strings -march=native -rdynamic -I$(NETLIB_INCLUDE)
LDFLAGS = -L$(NETLIB_LIBRARY) -lnetlib -lpthread

RAW_SOURCE = $(shell find -name '*.cc')
RAW = $(RAW_SOURCE:.cc=)

all: $(RAW)

.cc:
	$(CXX) -o $@ $< $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -f $(RAW) *~
