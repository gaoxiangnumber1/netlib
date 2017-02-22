NETLIB_DIRECTORY = $(HOME)/netlib
NETLIB_INCLUDE = $(NETLIB_DIRECTORY)/include
NETLIB_LIBRARY = $(NETLIB_DIRECTORY)/lib

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wconversion -Werror -Wextra -Wno-unused-parameter \
						-Wold-style-cast -Woverloaded-virtual -Wpointer-arith -Wshadow \
						-Wwrite-strings -march=native -rdynamic -I$(NETLIB_INCLUDE)
LDFLAGS = -L$(NETLIB_LIBRARY) -lnetlib -lpthread

SOURCE	= $(shell find -name '*.cc')
OBJECT	= $(SOURCE:.cc=.o)
ECHO_SERVER = single_thread_echo_server

all: $(ECHO_SERVER)

$(ECHO_SERVER): $(OBJECT)
	$(CXX) -o $(ECHO_SERVER) $(OBJECT) $(LDFLAGS)
.cc.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -f $(ECHO_SERVER) $(OBJECT) *~
