DEBUGFLAGS = -O3

all: magna
# ../pthreads-w32/pthread.o 
magna: magna.o genetic.o graph.o permutation.o carrier.o utils.o
	g++ $(DEBUGFLAGS) -o magna magna.o genetic.o graph.o permutation.o carrier.o utils.o -lpthread

magna.o: magna.cpp
	g++ $(DEBUGFLAGS) -Wall -c magna.cpp

genetic.o: genetic.cpp genetic.h
	g++ $(DEBUGFLAGS) -Wall -c genetic.cpp

graph.o: graph.cpp graph.h
	g++ $(DEBUGFLAGS) -Wall -c graph.cpp

permutation.o: permutation.cpp permutation.h
	g++ $(DEBUGFLAGS) -Wall -c permutation.cpp

carrier.o: carrier.cpp carrier.h
	g++ $(DEBUGFLAGS) -Wall -c carrier.cpp

utils.o: utils.cpp utils.h
	g++ $(DEBUGFLAGS) -Wall -c utils.cpp

CXX      = $(shell fltk-config --cxx) $(DEBUGFLAGS) 
CXXFLAGS = $(shell fltk-config --cxxflags ) -I. -I/usr/include $(DEBUGFLAGS) 
LDFLAGS  = $(shell fltk-config --ldflags )
LDSTATIC = $(shell fltk-config --ldstaticflags )
LINK     = $(CXX)

#TARGET = gui_magna
#OBJS = magna_gui.o genetic.o graph.o permutation.o carrier.o
#SRCS = magna_gui.cpp genetic.cpp graph.cpp permutation.cpp carrier.cpp
TARGET = magnafluidui
OBJS = magnaui.o magnauimain.o genetic.o graph.o permutation.o carrier.o utils.o
SRCS = magnaui.cpp magnauimain.cpp genetic.cpp graph.cpp permutation.cpp carrier.cpp utils.cpp

magnaui.o: magnaui.cpp magnaui.h magnauifluidadd.h magnauibase.h
magnauimain.o: magnauimain.cpp magnaui.h magnauifluidadd.h magnauibase.h

.SUFFIXES: .o .cpp .h
%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c $<

gui: $(TARGET)
	$(LINK) -o $(TARGET) $(OBJS) $(LDSTATIC)

$(TARGET): $(OBJS)

clean:
	rm -f *.o 2> /dev/null
	rm -f $(TARGET) $(TARGET).exe 2> /dev/null
	rm -f magna magna.exe

