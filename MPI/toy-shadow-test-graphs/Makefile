CC=mpicxx
CFLAGS=-std=c++11 -lstdc++ -g

SANA_SRC=\
	../../src/Graph.cpp\
	../../src/Alignment.cpp\
	../../src/arguments/GraphLoader.cpp\
	../../src/utils/FileIO.cpp\
	../../src/utils/Timer.cpp\
	../../src/utils/utils.cpp\
	../../src/utils/randomSeed.cpp

make: mt
mt:toy-shadow-test.cpp
	$(CC) $(CFLAGS) toy-shadow-test.cpp $(SANA_SRC) -o toy-shadow-test

old:
	$(CC) $(CFLAGS) toy-shadow-test-noshared.cpp $(SANA_SRC) -o toy-shadow-test-noshared
clean:
	rm -f *.exe.stackdump core.[0-9]*
	rm toy-shadow-test
	rm toy-shadow-test-noshared