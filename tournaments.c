#include "tournaments.h"

#define REMOVE_PLAYER(player_to_remove, winner_player)  (player_to_remove) = INVALID_ID;\
                                                        data->winner = winner_player

#define FIRST_PLACE(scorer) \
do {                        \
MapKeyElement best_player = mapGetFirst(scores); \
MAP_FOREACH(int*, player, players_by_id) {       \
if (scorer(scores, player) >= scorer(scores, best_player)) {\
best_player = player;\
}\
}\
MAP_FOREACH(int*, player, players_by_id) {\
if (scorer(scores, best_player)>scorer(scores, player)) {\
mapRemove(scores, player);\
}\
}\
if (mapGetSize(scores) == 1) {\
int winner = (int) mapGetFirst(scores);\
mapDestroy(scores);\
mapDestroy(players_by_id_copy);\
return winner;\
}\
}while(0)
typedef struct tournament_t {
    Map gameByBothPlayersId;
    int max_games_per_player;
    const char *location;
    int winner;
} *Tournament;

typedef struct game_key_t {
    int player_1_id;
    int player_2_id;
} *GameKey;


typedef struct game_t {
    Winner winner;
    int duration;
} *GameData;

GameKey copy_game_key(GameKey game_key) {
    ASSERT_NOT_NULL(game_key);
    GameKey new_key = malloc(sizeof(*new_key));
    RETURN_NULL_IF_NULL(new_key);
    new_key->player_1_id = game_key->player_1_id;
    new_key->player_2_id = game_key->player_2_id;
    return new_key;
}

GameData copy_game_data(GameData game_data) {
    ASSERT_NOT_NULL(game_data);
    GameData new_data = malloc(sizeof(*new_data));
    RETURN_NULL_IF_NULL(new_data);
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
    tournament->location = location;
    tournament->max_games_per_player = max_games_per_player;
    tournament->winner = 0;
}

static int getNumOfWinsAux(Map scores, int *player) {

}

static int getNumOfLosses(Map scores, int *player) {

}

void addPlayerStats(Map result, GameData this_game_data, int player_id) {

}

static Map getStatsByPlayer(Tournament tournament) {
    Map result = mapCreate(copyStatsFunc, copyInt, freeStatsFunc, freeInt, compareInt);
    NULL_ASSERT(result);
    MAP_FOREACH(GameKey, key, tournament->gameByBothPlayersId) {
        GameData this_game_data = mapGet(tournament->gameByBothPlayersId, key);
        if (mapContains(result, (MapKeyElement) key->player_1_id) == false) {
            struct play_stats_t stats;
            if (this_game_data->winner == FIRST_PLAYER) {
                stats.num_wins++;
            }
            if (this_game_data->winner == SECOND_PLAYER) {
                stats.num_losses++;
            }
            if (this_game_data->winner == DRAW) {
                stats.num_draws++;
            }
            mapPut(result, &key->player_1_id, &stats);
        }
        if (mapContains(result, (MapKeyElement) key->player_2_id) == false) {
            struct play_stats_t stats;
            if (this_game_data->winner == FIRST_PLAYER) {
                stats.num_losses++;
            }
            if (this_game_data->winner == SECOND_PLAYER) {
                stats.num_wins++;
            }
            if (this_game_data->winner == DRAW) {
                stats.num_draws++;
            }
            mapPut(result, &key->player_2_id, &stats);
        }
        addPlayerStats(result, this_game_data, key->player_1_id);
        addPlayerStats(result, this_game_data, key->player_2_id);
    }
    return result;
}

int calculateTournamentWinner(Map players_by_id, Tournament tournament) {
    Map players_by_id_copy = mapCopy(players_by_id);
    ASSERT_NOT_NULL(players_by_id);
    Map scores = mapCreate(&copyInt, &copyInt, &copyInt, &copyInt, &compareInt);
    RETURN_NULL_IF_NULL(scores);
    MAP_FOREACH(Map, player, players_by_id) {
        PlayerStats playerStats = tournamentGetPlayerStats(tournament, player);
        int score = (playerStats->num_draws) * 1 + (playerStats->num_wins) * 2;
        mapPut(scores, (MapKeyElement) player, (MapDataElement) score);
        int calculateTournamentWinner(Tournament tournament) {
            Map players_to_stats = getStatsByPlayer(tournament);
            CHECK_NULL(players_to_stats);
            Map scores = mapCreate(copyInt, copyInt, freeInt, freeInt, compareInt);
            CHECK_NULL(scores);
            MAP_FOREACH(int*, player, players_to_stats) {
                PlayerStats playerState = tournamentGetPlayerStats(tournament, *player);
                int score = (playerState->num_draws) * 1 + (playerState->num_wins) * 2;
                mapPut(scores, player, &score);
                freeStatsFunc(playerState);
            }

            FIRST_PLACE(mapGet);
            MapKeyElement least_losses = mapGetFirst(scores);
            MAP_FOREACH(int*, player, players_by_id) {
                if (getNumOfLosses(scores, least_losses) >= getNumOfLosses(scores, player)) {
                    least_losses = player;
                }
            }
            MAP_FOREACH(int*, player, players_by_id) {
                if (getNumOfLosses(scores, player) > getNumOfLosses(scores, least_losses)) {
                    mapRemove(scores, player);
                }
            }
            if (mapGetSize(scores) == 1) {
                int winner = (int) mapGetFirst(scores);
                mapDestroy(scores);
                mapDestroy(players_by_id_copy);
                return winner;
            }
            FIRST_PLACE(getNumOfWinsAux);
            int winner = (int) mapGetFirst(scores);
            mapDestroy(scores);
            mapDestroy(players_by_id_copy);
            return winner;
        }