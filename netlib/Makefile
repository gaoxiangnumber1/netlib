SRC = ../

CC = g++
CXXFLAGS = -std=c++11 -Wall -Wconversion -Werror -Wextra -Wno-unused-parameter \
						-Wold-style-cast -Woverloaded-virtual -Wpointer-arith -Wshadow \
						-Wwrite-strings -march=native -rdynamic \
						-I$(SRC)
LDFLAGS = -lpthread

.SUFFIXES: .cc .o

SOURCE =	logging.cc thread.cc time_stamp.cc timer.cc timer_queue.cc channel.cc poller.cc \
					event_loop.cc event_loop_thread.cc test.cc
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
