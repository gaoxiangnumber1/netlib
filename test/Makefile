INCLUDE = ../
SRC = ../netlib
CC = g++
CXXFLAGS = -std=c++11 -Wall -Wconversion -Werror -Wextra -Wno-unused-parameter \
						-Wold-style-cast -Woverloaded-virtual -Wpointer-arith -Wshadow \
						-Wwrite-strings -march=native -rdynamic \
						-I$(INCLUDE)
LDFLAGS = -lpthread

.SUFFIXES: .cc .o
SOURCE =	mutex_test.cc \
					thread_test.cc thread_bench.cc $(SRC)/thread.cc \
					$(SRC)/time_stamp.cc
OBJECT = $(SOURCE:.cc=.o)

MUTEX_TEST = mutex_test
MUTEX_OBJ = mutex_test.o $(SRC)/thread.o $(SRC)/time_stamp.o
THREAD_TEST = thread_test
THREAD_OBJ = thread_test.o $(SRC)/thread.o
THREAD_BENCH = thread_bench
THREAD_BENCH_OBJ = thread_bench.o $(SRC)/thread.o $(SRC)/time_stamp.o
TEST = $(MUTEX_TEST) $(THREAD_TEST) $(THREAD_BENCH)

all: $(OBJECT) $(TEST)

$(MUTEX_TEST): $(MUTEX_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(THREAD_TEST): $(THREAD_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(THREAD_BENCH): $(THREAD_BENCH_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

.cc.o:
	$(CC) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -f $(OBJECT) $(TEST) *~

.PHONY: all clean
