CC = g++

OPTM_FLAG = 3
CFLAGS = --std=c++20 -O$(OPTM_FLAG) -g -fopenmp

BINARIES = play_peg
GAME_DEPENDENCIES = board peg_solitaire
PARALLEL_ASTAR_IMPL = parallel_astar_lock parallel_astar_critical parallel_astar_task parallel_astar_calculate parallel_astar_fan
PARALLEL_ASTAR_DIR = ParallelAStar
AGENT_DEPENDENCIES = serial_search parallel_dbb serial_astar $(PARALLEL_ASTAR_IMPL)


OBJECTS = $(patsubst %, %.o, $(GAME_DEPENDENCIES) $(AGENT_DEPENDENCIES))

all: play_peg

VPATH = ./$(PARALLEL_ASTAR_DIR):./

play_peg: play_peg.o $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $< -o $@
	cp $@ ../$@


$(PARALLEL_ASTAR_DIR)/.cpp.o: 
	$(CC) $(CFLAGS) -c $(PARALLEL_ASTAR_DIR)/$<

.cpp.o:
	$(CC) $(CFLAGS) -c $<

clean:	
	rm -f *.o $(BINARIES) 
