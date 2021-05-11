CC = g++
CFLAGS = --std=c++20 -O0 -g -fopenmp

BINARIES = play_peg
GAME_DEPENDENCIES = board peg_solitaire
AGENT_DEPENDENCIES = serial_search parallel_dbb serial_astar parallel_astar

OBJECTS = $(patsubst %, %.o, $(GAME_DEPENDENCIES) $(AGENT_DEPENDENCIES))

all: play_peg

play_peg: play_peg.o $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

.cpp.o:
	$(CC) $(CFLAGS) -c $<

clean:	
	rm -f *.o $(BINARIES) 
