#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "map.h"
#include "chessSystem.h"
#include "tournaments.h"

#define CHECK_NULL(args) if ((args) == NULL) return NULL
#define NULL_ASSERT(args) assert(args != NULL)

typedef struct tournament_t {
    Map gameByBothPlayersId;
    int max_games_per_player;
    const char *tournament_location;
} *Tournament;

typedef struct game_key_t {
    int player_1_id;
    int player_2_id;
} *GameKey;


typedef struct game_t {//todo should it be here?
    Winner winner;
    int duration;
} *GameData;

GameKey copy_game_key(GameKey game_key) {
    NULL_ASSERT(game_key);
    GameKey new_key = malloc(sizeof(*new_key));
    CHECK_NULL(new_key);
    new_key->player_1_id = game_key->player_1_id;
    new_key->player_2_id = game_key->player_2_id;
    return new_key;
}

GameData copy_game_data(GameData game_data) {
    NULL_ASSERT(game_data);
    GameData new_data = malloc(sizeof(*new_data));
    CHECK_NULL(new_data);
    new_data->duration = game_data->duration;
    new_data->winner = game_data->winner;
    return new_data;
}

void free_game_data(GameData game_data) {
    free(game_data);
}

void free_game_key(GameKey game_key) {
    free(game_key)
}

int compare_game_key(GameKey game_key_1, GameKey game_key_2)
        Tournament tournamentCreate(int max_games_per_player, const char *location) {
    Tournament tournament = malloc(sizeof(*tournament));
    if (tournament == NULL) {
        return NULL;
    }
    tournament->gameByBothPlayersId = mapCreate(&copy_game_data, &copy_game_key, &free_game_data, &free_game_key,
                                                &compare_game_key);
    tournament->tournament_location = location;
    tournament->max_games_per_player = max_games_per_player;
}