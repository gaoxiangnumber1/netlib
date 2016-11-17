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
					$(SRC)/count_down_latch.cc \
					thread_test.cc thread_bench.cc $(SRC)/thread.cc \
					thread_pool_test.cc $(SRC)/thread_pool.cc\
					time_stamp_test.cc $(SRC)/time_stamp.cc \
					$(SRC)/timer.cc \
					logging_test.cc $(SRC)/logging.cc \
					$(SRC)/socket.cc \
					socket_address_test.cc $(SRC)/socket_address.cc \
					$(SRC)/socket_operation.cc \
					$(SRC)/epoller.cc \
					$(SRC)/channel.cc
OBJECT = $(SOURCE:.cc=.o)

MUTEX_TEST = mutex_test
MUTEX_OBJ = mutex_test.o $(SRC)/thread.o $(SRC)/time_stamp.o $(SRC)/logging.o
THREAD_TEST = thread_test
THREAD_OBJ = thread_test.o $(SRC)/logging.o $(SRC)/thread.o
THREAD_BENCH = thread_bench
THREAD_BENCH_OBJ = thread_bench.o $(SRC)/thread.o $(SRC)/time_stamp.o $(SRC)/logging.o
THREAD_POOL_TEST = thread_pool_test
THREAD_POOL_OBJ =		thread_pool_test.o $(SRC)/thread_pool.o $(SRC)/logging.o \
											$(SRC)/thread.o $(SRC)/count_down_latch.o
TIME_STAMP_TEST = time_stamp_test
TIME_STAMP_OBJ = time_stamp_test.o $(SRC)/time_stamp.o
LOG_TEST = logging_test
LOG_OBJ = logging_test.o $(SRC)/logging.o $(SRC)/thread.o
SOCKET_ADDRESS_TEST = socket_address_test
SOCKET_ADDRESS_OBJ =	socket_address_test.o $(SRC)/socket_address.o $(SRC)/logging.o \
												 $(SRC)/thread.o
TEST =	$(MUTEX_TEST) $(THREAD_TEST) $(THREAD_BENCH) \
				$(THREAD_POOL_TEST) $(TIME_STAMP_TEST) \
				$(LOG_TEST) $(SOCKET_ADDRESS_TEST)

all: $(OBJECT) $(TEST)

$(MUTEX_TEST): $(MUTEX_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(THREAD_TEST): $(THREAD_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(THREAD_BENCH): $(THREAD_BENCH_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(THREAD_POOL_TEST): $(THREAD_POOL_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(TIME_STAMP_TEST): $(TIME_STAMP_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(LOG_TEST): $(LOG_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(SOCKET_ADDRESS_TEST): $(SOCKET_ADDRESS_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

.cc.o:
	$(CC) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -f $(OBJECT) $(TEST) *~

.PHONY: all clean
