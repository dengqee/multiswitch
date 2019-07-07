GCC = g++
CFLAGS = -g -std=c++11
FILES = main.out

all: $(FILES)

main.out: main.o topology.o network.o
	$(GCC) $(CFLAGS) $^ -o main.out 
main.o: main.cpp topology.h network.h
	$(GCC) $(CFLAGS) -c main.cpp -o main.o
topology.o: topology.cpp topology.h
	$(GCC) $(CFLAGS) -c $< -o topology.o
network.o: network.cpp network.h
	$(GCC) $(CFLAGS) -c $< -o network.o
clean:
	rm -f *.o *.out
