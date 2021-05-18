#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "map.h"
#include "chessSystem.h"
#include "games.h"

typedef struct game_t {
    int player_1_id;
    int player_2_id;
    Winner winner;
    int duration;
} *Game;