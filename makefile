CC=gcc
OBJS=chessSystem.o util.o tournaments.o tests/chessSystemTestsExample.o
EXEC=chess
OBJ=chess.o
DEBUG=-g -DNDEBUG# now empty, assign -g for debug
CFLAGS=-std=c99 -Wall -pedantic-errors -Werror $(DEBUG)

$(EXEC) : $(OBJ)
	$(CC) $(CFLAG) $(OBJ) -o $@ -L. -lmap
$(OBJ): $(OBJS)
	ld -r -o $(OBJ) $(OBJS)

chessSystemTestsExample.o: tests/chessSystemTestsExample.c \
 chessSystem.h test_utilities.h
chessSystem.o: chessSystem.c chessSystem.h map.h tournaments.h
tournaments.o: tournaments.c tournaments.h map.h
util.o: util.h util.c chessSystem.h map.h
chessSystemTestsExample.o: chessSystemTestsExample.c chessSystem.h test_utilities.h

clean:
	rm -f $(OBJS) $(OBJ) $(EXEC)