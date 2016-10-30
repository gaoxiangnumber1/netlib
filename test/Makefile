SRC = ../src

CC = g++
CXXFLAGS = -std=c++11 -Wall -Wconversion -Werror -Wextra -Wno-unused-parameter \
						-Wold-style-cast -Woverloaded-virtual -Wpointer-arith -Wshadow \
						-Wwrite-strings -march=native -rdynamic \
						-I$(SRC)
LDFLAGS = -lpthread

.SUFFIXES: .cc .o

SOURCE =	$(SRC)/logging.cc logging_test.cc \
					$(SRC)/thread.cc thread_test.cc
OBJECT = $(SOURCE:.cc=.o)
LOG_TEST = logging_test
LOG_OBJ = logging_test.o $(SRC)/logging.o
THREAD_TEST = thread_test
THREAD_OBJ = thread_test.o $(SRC)/thread.o $(SRC)/logging.o
TEST =	$(LOG_TEST) \
				$(THREAD_TEST)

all: $(OBJECT) $(TEST)

$(LOG_TEST): $(LOG_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(THREAD_TEST): $(THREAD_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

.cc.o:
	$(CC) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -f $(OBJECT) $(TEST) *~ *.log

.PHONY: all clean
