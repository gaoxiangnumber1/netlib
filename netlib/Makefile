SRC = ../

CC = g++
CXXFLAGS = -std=c++11 -Wall -Wconversion -Werror -Wextra -Wno-unused-parameter \
						-Wold-style-cast -Woverloaded-virtual -Wpointer-arith -Wshadow \
						-Wwrite-strings -march=native -rdynamic \
						-I$(SRC)
LDFLAGS = -lpthread

.SUFFIXES: .cc .o
SOURCE =	test.cc \
					buffer.cc \
					tcp_connection.cc tcp_server.cc \
					acceptor.cc \
					event_loop.cc event_loop_thread.cc event_loop_thread_pool.cc \
					poller.cc \
					channel.cc \
					thread.cc \
					timer_queue.cc timer.cc time_stamp.cc \
					socket.cc socket_address.cc socket_operation.cc \
					logging.cc
OBJECT = $(SOURCE:.cc=.o)
TEST = test

all: $(OBJECT) $(TEST)

$(TEST): $(OBJECT)
	$(CC) -o $@ $^ $(LDFLAGS)

.cc.o:
	$(CC) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -f $(OBJECT) $(TEST) *~

.PHONY: all clean
