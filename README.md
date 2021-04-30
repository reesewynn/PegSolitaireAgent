# PegSolitaireAgent

## Background
This project was created by Reese Wynn in April 2021. 
The goal of this project is to analyze and compare serial and parallel implementations of search agents for the game Peg Solitaire.

## About Peg Solitaire
Peg Solitaire is a game over 300 years old played by royalty and citizens alike. It is a single player finite game consisting of 33 slots.
There are traditionally 32 slots filled with only a single empty slot in the center.
A peg may be "jumped" by an adjacent peg so long as the "jumping" peg moves into an empty slot opposite the "jumped" peg. 
This peg is now removed.
The game is over when a single peg remains in the center (a win) or there are no other moves (a loss). 
More information can be found at:
https://en.wikipedia.org/wiki/Peg_solitaire

## Included Code
peg_solitaire.hpp - Describes a board of peg solitaire. This represents the board state, legal moves, and the detection of a win or loss state.
play_peg.hpp - Can run a game of peg solitaire with the following search algorithms:
  1. Backtracking (SERIAL)
