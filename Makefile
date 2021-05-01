CC = g++
CFLAGS = --std=c++20 -O0 -g 

BINARIES = play_peg

all: play_peg

play_peg: play_peg.o board.o peg_solitaire.o serial_search.o
	$(CC) $(CFLAGS) $^ -o $@

# peg_solitaire: board.o
# 		$(CC) $(CFLAGS) -c $^

.cpp.o:
	$(CC) $(CFLAGS) -c $<

clean:	
	rm -f *.o $(BINARIES) 
