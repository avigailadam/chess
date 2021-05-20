.DEFAULT_GOAL := chess
CC = gcc
FLAGS = -std=c99 -pedantic-errors -Werror -DNDEBUG
chess: chessSystem test_utilities.h
	$(CC) $(FLAGS) -no-pie tests/chessSystemTestsExample.c *.o *.a *.h -o chess
chessSystem: chessSystem.h tournaments util
	$(CC) $(FLAGS) -c chessSystem.c
tournaments: tournaments.h util
	$(CC) $(FLAGS) -c tournaments.c
util: util.h
	$(CC) $(FLAGS) -c util.c

clean:
	rm *.o