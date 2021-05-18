#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "map.h"
#include "tournaments.h"

typedef struct tournament_t {
    Map gameByBothPlayersId;
    int max_games_per_player;
    const char *tournament_location;
} *Tournament;

typedef struct game_key_t {
    int player_1_id;
    int player_2_id;
} *GameKey;


Tournament tournamentCreate(int max_games_per_player, const char *location) {
    return NULL;
}