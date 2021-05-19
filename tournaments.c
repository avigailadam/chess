#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "map.h"
#include "chessSystem.h"
#include "tournaments.h"
#include "util.h"

#define CHECK_NULL(args) if ((args) == NULL) return NULL
#define NULL_ASSERT(args) assert(args != NULL)

typedef struct tournament_t {
    Map gameByBothPlayersId;
    int max_games_per_player;
    const char *tournament_location;
    int winner;
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
    free(game_key);
}

int compare_game_key(GameKey game_key_1, GameKey game_key_2) {
    int player_1_diff = game_key_1->player_1_id - game_key_2->player_1_id;
    int player_2_diff = game_key_1->player_2_id - game_key_2->player_2_id;
    return player_1_diff + player_2_diff;
}

Tournament tournamentCreate(int max_games_per_player, const char *location) {
    Tournament tournament = malloc(sizeof(*tournament));
    if (tournament == NULL) {
        return NULL;
    }
    tournament->gameByBothPlayersId = mapCreate(&copy_game_data, &copy_game_key, &free_game_data, &free_game_key,
                                                &compare_game_key);
    tournament->tournament_location = location;
    tournament->max_games_per_player = max_games_per_player;
    tournament->winner = 0;
}

//void addToValue(Map scores, int player_id, int add) {
//    if (!mapContains(scores, (MapKeyElement) (player_id))) {
//        mapPut(scores, (MapKeyElement) player_id, &0);
//    }
//    int *old_value = mapGet(scores, (MapKeyElement) player_id);
//    *old_value += add;
//    mapPut(scores, (MapKeyElement) player_id, old_value);
//}

int *creatScoreTable(Map scores, int num_of_players) {
    int *table = (int *) malloc(sizeof(int) * num_of_players);
    int count = 0;
    MAP_FOREACH(Map, player, scores) {
        table[count] = (int) mapGet(scores, player);
        count++;
    }


    return table;
}

int calculateTournamentWinner(Map players_by_id, Tournament tournament) {
    Map players_by_id_copy = mapCopy(players_by_id);
    NULL_ASSERT(players_by_id);
    Map scores = mapCreate(&copyInt, &copyInt, &copyInt, &copyInt, &compareInt);
    CHECK_NULL(scores);
    MAP_FOREACH(Map, player, players_by_id) {
        PlayerStats playerStats = tournamentGetPlayerStats(tournament, player);
        int score = (playerStats->num_draws) * 1 + (playerStats->num_wins) * 2;
        mapPut(scores, (MapKeyElement) player, (MapDataElement) score);
    }
    MapKeyElement best_player = mapGetFirst(scores);
    MAP_FOREACH(Map, player, players_by_id) {
        if (mapGet(scores, player) >= mapGet(scores, best_player)) {
            best_player = player;
        }
    }
    MAP_FOREACH(Map, player, players_by_id) {
        if (mapGet(scores, player) < mapGet(scores, best_player)) {
            mapRemove(scores, player);
        }
    }
    if (mapGetSize(scores) == 1) {
        int winner = (int) mapGetFirst(scores);
        mapDestroy(scores);
        mapDestroy(players_by_id_copy);
        return winner;
    }
    MapKeyElement least_losses = mapGetFirst(scores);
    MAP_FOREACH(Map, player, players_by_id) {
        if (getNumOfLosses((int) least_losses) >= getNumOfLosses((int) player)) {
            least_losses = player;
        }
    }
    MAP_FOREACH(Map, player, players_by_id) {
        if (getNumOfLosses((int) least_losses) < getNumOfLosses((int) player)) {
            mapRemove(scores, player);
        }
    }
    if (mapGetSize(scores) == 1) {
        int winner = (int) mapGetFirst(scores);
        mapDestroy(scores);
        mapDestroy(players_by_id_copy);
        return winner;
    }
    MapKeyElement most_wins = mapGetFirst(scores);
    MAP_FOREACH(Map, player, players_by_id) {
        if (getNumOfWins((int) most_wins) <= getNumOfWins((int) player)) {
            most_wins = player;
        }
    }
    MAP_FOREACH(Map, player, players_by_id) {
        if (getNumOfWins((int) most_wins) > getNumOfWins((int) player)) {
            mapRemove(scores, player);
        }
    }
    if (mapGetSize(scores) == 1) {
        int winner = (int) mapGetFirst(scores);
        mapDestroy(scores);
        mapDestroy(players_by_id_copy);
        return winner;
    }
    int winner = (int) mapGetFirst(scores);
    mapDestroy(scores);
    mapDestroy(players_by_id_copy);
    return winner;
}
/* Map countWinsByPlayers = mapCreate(copyInt, copyInt, freeInt, freeInt, compareInt);
 MAP_FOREACH(GameKey, gameKey, tournament->gameByBothPlayersId) {
     GameData data = mapGet(tournament->gameByBothPlayersId, gameKey);
     CHECK_NULL(data);
     int player_1_score_change = 0;
     int player_2_score_change = 0;

     if (data->winner == FIRST_PLAYER) {
         player_1_score_change = 2;
     }
     if (data->winner == SECOND_PLAYER)
         player_2_score_change = 2;;
     if (data->winner == DRAW) {
         player_1_score_change = 1;
         player_2_score_change = 1;
     }
     addToValue(countWinsByPlayers, gameKey->player_1_id, player_1_score_change);
     addToValue(countWinsByPlayers, gameKey->player_2_id, player_2_score_change);
 }
 winner=
 MAP_FOREACH(MapKeyElement , players, countWinsByPlayers){
     if(winner>=)
 }

     mapDestroy(countWinsByPlayers);
     return winner;
 }
 */