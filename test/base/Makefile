NETLIB_SRC = ../../src

CC = g++
CXXFLAGS = -std=c++11 -Wall -Wconversion -Werror -Wextra -Wno-unused-parameter \
						-Wold-style-cast -Woverloaded-virtual -Wpointer-arith -Wshadow \
						-Wwrite-strings -march=native -rdynamic \
						-I$(NETLIB_SRC)

.SUFFIXES: .cc .o

SOURCE = atomic_test.cc types_test.cc
OBJECT = $(SOURCE:.cc=.o)
ATOMIC_TEST = atomic_test
ATOMIC_OBJ = atomic_test.o
TYPES_TEST = types_test
TYPES_OBJ = types_test.o
ALL_TEST = $(ATOMIC_TEST) $(TYPES_TEST)

all: $(ALL_TEST)

$(ATOMIC_TEST): $(ATOMIC_OBJ)
	$(CC) -o $@ $^

$(TYPES_TEST): $(TYPES_OBJ)
	$(CC) -o $@ $^

.cc.o:
	$(CC) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -f $(OBJECT) $(ALL_TEST) *~

.PHONY: all clean
