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
SINGLE = single_thread_sudoku
SINGLE_OBJECT = single_thread_sudoku.o sudoku.o
THREAD_POOL = thread_pool_sudoku
THREAD_POOL_OBJECT = thread_pool_sudoku.o sudoku.o
MULTILOOP = multiloop_sudoku
MULTILOOP_OBJECT = multiloop_sudoku.o sudoku.o
MULTILOOP_THREAD_POOL = multiloop_thread_pool_sudoku
MULTILOOP_THREAD_POOL_OBJECT = multiloop_thread_pool_sudoku.o sudoku.o
TARGET =	$(SINGLE) $(THREAD_POOL) $(MULTILOOP) \
					$(MULTILOOP_THREAD_POOL)

all: $(TARGET)

$(SINGLE): $(OBJECT)
	$(CXX) -o $(SINGLE) $(SINGLE_OBJECT) $(LDFLAGS)
$(THREAD_POOL): $(OBJECT)
	$(CXX) -o $(THREAD_POOL) $(THREAD_POOL_OBJECT) $(LDFLAGS)
$(MULTILOOP): $(OBJECT)
	$(CXX) -o $(MULTILOOP) $(MULTILOOP_OBJECT) $(LDFLAGS)
$(MULTILOOP_THREAD_POOL): $(OBJECT)
	$(CXX) -o $(MULTILOOP_THREAD_POOL) $(MULTILOOP_THREAD_POOL_OBJECT) $(LDFLAGS)
.cc.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -f $(TARGET) $(OBJECT) *~
