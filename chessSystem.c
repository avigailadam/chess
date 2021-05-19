#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <assert.h>
#include "map.h"
#include "chessSystem.h"
#include "util.h"
#include "tournaments.h"

#define MIN_LOCATION_LEN 3

#define PRINT_INT_TO_FILE(func, arg_to_func, arg, file) arg = func(arg_to_func);\
        assert(arg >= 0);\
        fprintf(file, "%d\n", arg)

typedef int TournamentId;

typedef struct chess_system_t {
    Map tournamentsById;
} *ChessSystem;

Tournament tournamentCreate(int max_games_per_player, const char *location);

bool locationIsValid(const char *location) {
    if (location == NULL || strlen(location) < MIN_LOCATION_LEN) {
        return false;
    }
    if (!(location[0] >= 'A' && location[0] <= 'Z')) {
        return false;
    }
    for (int i = 1; i < strlen(location) - 1; ++i) {
        if (location[i] != ' ' && !(location[i] >= 'a' && location[i] <= 'z')) {
            return false;
        }
    }
    return true;
}

ChessResult convertResults(MapResult result) {
    if (result == MAP_SUCCESS) {
        return CHESS_SUCCESS;
    }
    if (result == MAP_NULL_ARGUMENT) {
        return CHESS_NULL_ARGUMENT;
    }
    if (result == MAP_ITEM_ALREADY_EXISTS) {
        return CHESS_TOURNAMENT_ALREADY_EXISTS;
    }
    if (result == MAP_ITEM_DOES_NOT_EXIST) {
        return CHESS_TOURNAMENT_NOT_EXIST;
    }
    if (result == MAP_OUT_OF_MEMORY) {
        return CHESS_OUT_OF_MEMORY;
    }
    assert(0);
}

ChessSystem chessCreate() {
    ChessSystem result = malloc(sizeof(*result));
    CHECK_NULL(result);
    result->tournamentsById = mapCreate(&copyTournament, &copyInt, &freeTournament, &freeInt,
                                        &compareInt);
    CHECK_NULL(result->tournamentsById);
    return result;
}

void chessDestroy(ChessSystem chess) {
    if (chess == NULL) {
        return;
    }
    MAP_FOREACH(Map, tournament, chess->tournamentsById) {
        mapDestroy(tournament);
    }
    free(chess);
}

ChessResult chessRemoveTournament(ChessSystem chess, int tournament_id) {
    if (chess == NULL) {
        return CHESS_NULL_ARGUMENT;
    }
    if (tournament_id <= 0) {
        return CHESS_INVALID_ID;
    }
    MAP_FOREACH(Map, tournament, chess->tournamentsById) {
        if (mapContains(tournament, (MapKeyElement) tournament_id)) {
            updateStatisticsForTournament(tournament);
            mapDestroy(tournament);
            return CHESS_SUCCESS;
        }
        return CHESS_TOURNAMENT_NOT_EXIST;
    }
}

ChessResult chessEndTournament(ChessSystem chess, int tournament_id) {
    if (chess == NULL) {
        return CHESS_NULL_ARGUMENT;
    }
    if (tournament_id <= 0) {
        return CHESS_INVALID_ID;
    }

    MapDataElement tournament = mapGet(chess->tournamentsById, (MapKeyElement) tournament_id);
    if (tournament == NULL) {
        return CHESS_TOURNAMENT_NOT_EXIST;
    }
    int winner = calculateTournamentWinner(, tournament);
    return CHESS_SUCCESS;
}

ChessResult chessRemovePlayer(ChessSystem chess, int player_id) {
    if (chess == NULL) {
        return CHESS_NULL_ARGUMENT;
    }
    if (player_id <= 0) {
        return CHESS_INVALID_ID;
    }
    bool was_removed = false;
    MAP_FOREACH(int*, tournamentId, chess->tournamentsById) {
        Tournament tournament = mapGet(chess->tournamentsById, &tournamentId);
        NULL_ASSERT(tournament);
        if (tournamentHasEnded(tournament)) {
            continue;
        }
        was_removed = was_removed || tournamentRemovePlayer(tournament, player_id);
    }
    return was_removed ? CHESS_SUCCESS : CHESS_PLAYER_NOT_EXIST;
}

ChessResult chessAddTournament(ChessSystem chess, int tournament_id,
                               int max_games_per_player, const char *tournament_location) {
    if (chess == NULL) {
        return CHESS_NULL_ARGUMENT;
    }
    if (tournament_id <= 0) {
        return CHESS_INVALID_ID;
    }
    if (!locationIsValid(tournament_location)) {
        return CHESS_INVALID_LOCATION;
    }
    if (mapContains(chess->tournamentsById, (MapKeyElement) &tournament_id) == true) {
        return CHESS_TOURNAMENT_ALREADY_EXISTS;
    }
    if (max_games_per_player <= 0) {
        return CHESS_INVALID_MAX_GAMES;
    }
    Tournament tournament = tournamentCreate(max_games_per_player, tournament_location);
    return tournament == NULL
           ? CHESS_OUT_OF_MEMORY
           : convertResults(
                    mapPut(chess->tournamentsById, (MapKeyElement) &tournament_id, (MapDataElement) tournament));
}

ChessResult chessAddGame(ChessSystem chess, int tournament_id, int first_player,
                         int second_player, Winner winner, int play_time) {
    if (chess == NULL) {
        return CHESS_NULL_ARGUMENT;
    }
    if (tournament_id <= 0 || second_player == first_player || second_player <= 0 || first_player <= 0) {
        return CHESS_INVALID_ID;
    }
    if (mapContains(chess->tournamentsById, (MapKeyElement) &tournament_id) == false) {
        return CHESS_TOURNAMENT_NOT_EXIST;
    }
    if (doesGameExist(first_player, second_player)) {
        return CHESS_GAME_ALREADY_EXISTS;
    }
    if (play_time < 0) {
        return CHESS_INVALID_PLAY_TIME;
    }
    return gameCreate(first_player, second_player, winner, play_time);
}

double chessCalculateAveragePlayTime(ChessSystem chess, int player_id, ChessResult *chess_result) {
    if (chess == NULL) {
        *chess_result = CHESS_NULL_ARGUMENT;
        return 0;
    }
    if (player_id <= 0) {
        *chess_result = CHESS_INVALID_ID;
        return 0;
    }
    int count = 0;
    double total_play_time = 0;
    MAP_FOREACH(int*, tournament_id, chess->tournamentsById) {
        Tournament tournament = mapGet(chess->tournamentsById, &tournament_id);
        NULL_ASSERT(tournament);
        PlayerStats player_stats = tournamentGetPlayerStats(tournament, player_id);
        if (player_stats == NULL)
            continue;
        count += player_stats->num_wins + player_stats->num_losses + player_stats->num_draws;
        total_play_time += player_stats->total_play_time;
    }
    if (count == 0) {
        *chess_result = CHESS_PLAYER_NOT_EXIST;
        return 0;
    }
    *chess_result = CHESS_SUCCESS;
    return total_play_time / count;
}

ChessResult chessSavePlayersLevels(ChessSystem chess, FILE *file) {
    Map playersStats = mapCreate(&copyStatsFunc, &copyInt, &freeStatsFunc, freeInt,
                                 compareInt); //key:player_id, data:player stats
    MAP_FOREACH(int*, tournament_id, chess->tournamentsById) {
        Tournament tournament = mapGet(chess->tournamentsById, &tournament_id);
        NULL_ASSERT(tournament);
        ChessResult result = tournamentUpdatePlayerStats(tournament, playersStats);
        if (result != CHESS_SUCCESS) {
            return result;
        }
    }
    MAP_FOREACH(int*, player_id, playersStats) {
        PlayerStats stats = mapGet(playersStats, player_id);
        NULL_ASSERT(stats);
        double level = (double) (6 * stats->num_wins - 10 * stats->num_losses + 2 * stats->num_draws) /
                       (stats->num_draws + stats->num_losses + stats->num_wins);
        if (fprintf(file, "%d %2f\n", *player_id, level) < 0) {
            return CHESS_SAVE_FAILURE;
        }
    }
    return CHESS_SUCCESS;
}

ChessResult chessSaveTournamentStatistics(ChessSystem chess, char *path_file) {
    FILE *file = fopen(path_file, "w");
    bool tournament_has_ended = false;
    MAP_FOREACH(int*, tournament_key, chess->tournamentsById) {
        Tournament tournament = mapGet(chess->tournamentsById, tournament_key);
        NULL_ASSERT(tournament);
        if (tournamentHasEnded(tournament) == false) {
            continue;
        }
        tournament_has_ended = true;
        int tmp;
        PRINT_INT_TO_FILE(getWinner, tournament, tmp, file);
        PRINT_INT_TO_FILE(longestGameTime, tournament, tmp, file);
        PRINT_INT_TO_FILE(averageGameTime, tournament, tmp, file);
        const char *location = getLocation(tournament);
        assert(location != NULL);
        fprintf(file, "%s\n", location);
        PRINT_INT_TO_FILE(getNumberOfPlayers, tournament, tmp, file);
        PRINT_INT_TO_FILE(getNumberOfGames, tournament, tmp, file);
    }
    return tournament_has_ended ? CHESS_SUCCESS : CHESS_NO_TOURNAMENTS_ENDED;
}
