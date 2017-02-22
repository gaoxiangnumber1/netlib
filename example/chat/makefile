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
SINGLE = single_thread_chat_server
SINGLE_OBJECT = single_thread_chat_server.o codec.o
MULTI = multithread_chat_server
MULTI_OBJECT = multithread_chat_server.o codec.o
COW = multithread_cow_chat_server
COW_OBJECT = multithread_cow_chat_server.o codec.o
CLIENT = chat_client
CLIENT_OBJECT = chat_client.o codec.o
TARGET =	$(OBJECT) $(SINGLE) $(MULTI) $(COW) $(CLIENT)

all: $(TARGET)

$(SINGLE):
	$(CXX) -o $(SINGLE) $(SINGLE_OBJECT) $(CXXFLAGS) $(LDFLAGS)
$(MULTI):
	$(CXX) -o $(MULTI) $(MULTI_OBJECT) $(CXXFLAGS) $(LDFLAGS)
$(COW):
	$(CXX) -o $(COW) $(COW_OBJECT) $(CXXFLAGS) $(LDFLAGS)
$(CLIENT):
	$(CXX) -o $(CLIENT) $(CLIENT_OBJECT) $(CXXFLAGS) $(LDFLAGS)

.cc.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -f $(TARGET) *~
