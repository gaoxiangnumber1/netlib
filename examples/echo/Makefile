MUDUO_DIRECTORY = $(HOME)/build/release-install
MUDUO_INCLUDE = $(MUDUO_DIRECTORY)/include
MUDUO_LIBRARY = $(MUDUO_DIRECTORY)/lib

CC = g++
CXXFLAGS = -g -O0 -Wall -Wextra -Werror \
	   -Wconversion -Wno-unused-parameter \
	   -Wold-style-cast -Woverloaded-virtual \
	   -Wpointer-arith -Wshadow -Wwrite-strings \
	   -march=native -rdynamic \
	   -I$(MUDUO_INCLUDE)
LDFLAGS = -L$(MUDUO_LIBRARY) -lmuduo_net -lmuduo_base -lpthread #-lrt

.SUFFIXES: .cc .o

SOURCE = main.cc echo.cc
OBJECT = $(SOURCE:.cc=.o)
EXEC = echo-server

all:  echo-server

$(EXEC): $(OBJECT)
	$(CC) -o $(EXEC) $(OBJECT) $(LDFLAGS)

.cc.o:
	$(CC) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -f $(OBJECT) *~

.PHONY: all clean
