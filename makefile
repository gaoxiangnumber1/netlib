CXX = g++
CXXFLAG =	-std=c++11 -Wall -Wconversion -Werror -Wextra -Wno-unused-parameter \
						-Wold-style-cast -Woverloaded-virtual -Wpointer-arith -Wshadow \
						-Wwrite-strings -march=native -rdynamic -I.

NETLIB_SOURCE	= $(shell find netlib -name '*.cc')
NETLIB_OBJECT	= $(NETLIB_SOURCE:.cc=.o)

LIBRARY = libnetlib.a

all: $(LIBRARY)

install: $(LIBRARY)
	mkdir -p $(HOME)/netlib/include/netlib $(HOME)/netlib/lib
	cp -f netlib/*.h $(HOME)/netlib/include/netlib
	cp -f $(LIBRARY) $(HOME)/netlib/lib
	rm -f $(LIBRARY) $(NETLIB_OBJECT) *~
uninstall:
	rm -rf $(HOME)/netlib/
clean:
	rm -f $(LIBRARY) $(NETLIB_OBJECT) *~

$(LIBRARY): $(NETLIB_OBJECT)
	ar -rcs $@ $(NETLIB_OBJECT)

.cc.o:
	$(CXX) $(CXXFLAG) -o $@ -c $<
