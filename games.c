#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "map.h"
#include "chessSystem.h"
#include "games.h"

typedef struct game_t {
    Winner winner;
    int duration;
} *Game;