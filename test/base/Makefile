NETLIB_SRC = ../../src
BASE = ../../src/base

CC = g++
CXXFLAGS = -std=c++11 -Wall -Wconversion -Werror -Wextra -Wno-unused-parameter \
						-Wold-style-cast -Woverloaded-virtual -Wpointer-arith -Wshadow \
						-Wwrite-strings -march=native -rdynamic \
						-I$(NETLIB_SRC)

.SUFFIXES: .cc .o

SOURCE =	atomic_test.cc \
					types_test.cc \
					$(BASE)/date.cc date_test.cc \
					$(BASE)/time_zone.cc time_zone_test.cc \
					$(BASE)/time_stamp.cc time_stamp_test.cc
OBJECT = $(SOURCE:.cc=.o)
ATOMIC_TEST = atomic_test
ATOMIC_OBJ = atomic_test.o
TYPES_TEST = types_test
TYPES_OBJ = types_test.o
DATE_TEST = date_test
DATE_OBJ = date_test.o $(BASE)/date.o
TIME_ZONE_TEST = time_zone_test
TIME_ZONE_OBJ = time_zone_test.o $(BASE)/time_zone.o $(BASE)/date.o
TIME_STAMP_TEST = time_stamp_test
TIME_STAMP_OBJ = time_stamp_test.o $(BASE)/time_stamp.o
TEST =	$(ATOMIC_TEST) \
				$(TYPES_TEST) \
				$(DATE_TEST) \
				$(TIME_ZONE_TEST) \
				$(TIME_STAMP_TEST)

all: $(OBJECT) $(TEST)

$(ATOMIC_TEST): $(ATOMIC_OBJ)
	$(CC) -o $@ $^

$(TYPES_TEST): $(TYPES_OBJ)
	$(CC) -o $@ $^

$(DATE_TEST): $(DATE_OBJ)
	$(CC) -o $@ $^

$(TIME_ZONE_TEST): $(TIME_ZONE_OBJ)
	$(CC) -o $@ $^

$(TIME_STAMP_TEST): $(TIME_STAMP_OBJ)
	$(CC) -o $@ $^

.cc.o:
	$(CC) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -f $(OBJECT) $(TEST) *~

.PHONY: all clean
