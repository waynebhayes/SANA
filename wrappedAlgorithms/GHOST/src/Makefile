CXX = g++
CXXFLAGS = -O3 -std=c++11

HEADERS = $(wildcard src/*.hpp)

BOOSTDIR ?= /usr/local/include
BOOSTLIB ?= /usr/local/lib
LFLAGS = -lboost_iostreams -lboost_system -lboost_thread -lpthread -lz -lrt
INCLUDES = -I include -I $(BOOSTDIR) -L $(BOOSTLIB) $(LFLAGS)

GHOST: src/GHOST.cpp $(HEADERS)
	$(CXX) src/GHOST.cpp $(CXXFLAGS) $(INCLUDES) -o $@

clean:
	\rm -f GHOST

tar:
	tar cfv *.sig.gz *.sdf *.af
