GCC = g++
CFLAGS = -pthread -g -std=c++11
FILES = main.out
LIB = main.o topology.o network.o plasement-problem.o problem-base.o \
	  measure-assignment-problem.o

all: $(FILES)

main.out: $(LIB) 
	$(GCC) $(CFLAGS) $^ -o main.out 

main.o: main.cpp topology.h network.h
	$(GCC) $(CFLAGS) -c main.cpp -o main.o

topology.o: topology.cpp topology.h
	$(GCC) $(CFLAGS) -c $< -o topology.o

network.o: network.cpp network.h
	$(GCC) $(CFLAGS) -c $< -o network.o

plasement-problem.o: plasement-problem.cpp plasement-problem.h 
	$(GCC) $(CFLAGS) -c $< -o plasement-problem.o

problem-base.o: problem-base.cpp problem-base.h
	$(GCC) $(CFLAGS) -c $< -o problem-base.o

measure-assignment-problem.o: measure-assignment-problem.cpp measure-assignment-problem.h
	$(GCC) $(CFLAGS) -c $< -o measure-assignment-problem.o

clean:
	rm -f *.o *.out
