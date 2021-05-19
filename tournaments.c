#include "tournaments.h"

#define REMOVE_PLAYER(player_to_remove, winner_player)  (player_to_remove) = INVALID_ID;\
                                                        data->winner = winner_player

#define FIRST_PLACE(scorer) \
do {                        \
MapKeyElement best_player = mapGetFirst(scores); \
MAP_FOREACH(int*, player, scores) {       \
if (scorer(scores, player) >= scorer(scores, best_player)) {\
best_player = player;\
}\
}\
MAP_FOREACH(int*, player, scores) {\
if (scorer(scores, best_player)>scorer(scores, player)) {\
mapRemove(scores, player);\
}\
}\
if (mapGetSize(scores) == 1) {\
int winner = (int) mapGetFirst(scores);\
mapDestroy(scores);\
mapDestroy(players_to_stats);\
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
    tournament->gameByBothPlayersId = mapCreate((copyMapDataElements) &copy_game_data,
                                                (copyMapKeyElements) &copy_game_key,
                                                (freeMapDataElements) &free_game_data,
                                                (freeMapKeyElements) &free_game_key,
                                                (compareMapKeyElements) &compare_game_key);
    tournament->location = location;
    tournament->max_games_per_player = max_games_per_player;
    tournament->winner = INVALID_ID;
}

static int getNumOfWinsAux(Map scores, int *player) {
    assert(scores != NULL);
    PlayerStats stats = mapGet(scores, &player);
    ASSERT_NOT_NULL(stats);
    int result = stats->num_wins;
    freeStatsFunc(stats);
    return result;
}

static int getNumOfLosses(Map scores, int *player) {
    assert(scores != NULL);
    PlayerStats stats = mapGet(scores, &player);
    ASSERT_NOT_NULL(stats);
    int result = stats->num_losses;
    freeStatsFunc(stats);
    return result;
}

void addPlayerStats(Map stats_by_players, GameData this_game_data, int player_id_winner, int player_id_looser) {
    assert(stats_by_players != NULL);
    PlayerStats stats_winner = mapGet(stats_by_players, &player_id_winner);
    ASSERT_NOT_NULL(stats_winner);
    stats_winner->num_wins++;
    mapPut(stats_by_players, &player_id_winner, stats_winner);
    freeStatsFunc(stats_winner);
    PlayerStats stats_looser = mapGet(stats_by_players, &player_id_looser);
    ASSERT_NOT_NULL(stats_looser);
    stats_looser->num_losses++;
    mapPut(stats_by_players, &player_id_looser, stats_looser);
    freeStatsFunc(stats_looser);
}

void addEmptyStatsIfNotExists(Map states_per_player, int player_id) {

}


static Map getStatsByPlayer(Tournament tournament) {
    Map result = mapCreate((copyMapDataElements) &copyStatsFunc, (copyMapKeyElements) &copyInt,
                           (freeMapDataElements) &freeStatsFunc, (freeMapKeyElements) &freeInt,
                           (compareMapKeyElements) &compareInt);
    ASSERT_NOT_NULL(result);
    MAP_FOREACH(GameKey, key, tournament->gameByBothPlayersId) {
        GameData this_game_data = mapGet(tournament->gameByBothPlayersId, key);
        addEmptyStatsIfNotExists(result, key->player_1_id);
        addEmptyStatsIfNotExists(result, key->player_2_id);
        switch (this_game_data->winner) {
            case FIRST_PLAYER:
                addPlayerStats(result, this_game_data, key->player_1_id, key->player_2_id);
                break;
            case SECOND_PLAYER:
                addPlayerStats(result, this_game_data, key->player_2_id, key->player_1_id);
                break;
            case DRAW:
                PlayerStats stats = mapGet(result, (MapKeyElement) key->player_1_id);
                ASSERT_NOT_NULL(stats);
                stats->num_draws++;
                mapPut(result, (MapKeyElement) key->player_1_id, stats);
                freeStatsFunc(stats);
                PlayerStats stats_2 = mapGet(result, (MapKeyElement) key->player_1_id);
                ASSERT_NOT_NULL(stats_2);
                stats_2->num_draws++;
                mapPut(result, (MapKeyElement) key->player_1_id, stats_2);
                freeStatsFunc(stats_2);
                break;
        }
        if (this_game_data->winner == FIRST_PLAYER) {
            addPlayerStats(result, this_game_data, key->player_1_id, FIRST_PLAYER);
            continue;
        }
        if (this_game_data->winner == SECOND_PLAYER) {
            addPlayerStats(result, this_game_data, key->player_2_id, SECOND_PLAYER);
            continue;
        }
        addPlayerStats(result, this_game_data, key->player_2_id, DRAW);
    }
    return result;
}

int calculateTournamentWinner(Tournament tournament) {
    Map players_to_stats = getStatsByPlayer(tournament);
    if (mapGetSize(players_to_stats) == 0) {
        return INVALID_ID;
    }
    Map scores;
    scores = mapCreate((copyMapDataElements) &copyInt, (copyMapKeyElements) &copyInt, (freeMapDataElements) &freeInt,
                       (freeMapKeyElements) &freeInt, (compareMapKeyElements) &compareInt);
    RETURN_NULL_IF_NULL(scores);
    MAP_FOREACH(int*, player, players_to_stats) {
        PlayerStats playerState = tournamentGetPlayerStats(tournament, *player);
        int score = (playerState->num_draws) * 1 + (playerState->num_wins) * 2;
        mapPut(scores, player, &score);
        freeStatsFunc(playerState);
    }

    FIRST_PLACE(mapGet);
    MapKeyElement least_losses = mapGetFirst(scores);
    MAP_FOREACH(int*, player, players_to_stats) {
        if (getNumOfLosses(scores, least_losses) >= getNumOfLosses(scores, player)) {
            least_losses = player;
        }
    }
    MAP_FOREACH(int*, player, players_to_stats) {
        if (getNumOfLosses(scores, player) > getNumOfLosses(scores, least_losses)) {
            mapRemove(scores, player);
        }
    }
    if (mapGetSize(scores) == 1) {
        int winner = (int) mapGetFirst(scores);
        mapDestroy(scores);
        mapDestroy(players_to_stats);
        return winner;
    }
    FIRST_PLACE(getNumOfWinsAux);
    int winner = (int) mapGetFirst(scores);
    mapDestroy(scores);
    mapDestroy(players_to_stats);
    return winner;
}