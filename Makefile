CC = g++
CFLAGS = --std=c++20 -O3 -g -fopenmp

BINARIES = play_peg
SRC_DIR = "./src"

all: play_peg

play_peg: FORCE
	cd $(SRC_DIR) && $(MAKE)

clean:	
	cd $(SRC_DIR) && $(MAKE) clean
	rm -f $(BINARIES)

FORCE: ;