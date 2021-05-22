#include <string.h>
#include "tournaments.h"

#define MIN_LOCATION_LEN 1

#define PRINT_INT_TO_FILE(func, arg_to_func, arg, file) arg = func(arg_to_func);\
        assert(arg >= 0);\
        fprintf(file, "%d\n", arg)

#define FOREACH_TOURNAMENT MAP_FOREACH_VALUE(int*, tournament_id, Tournament, tournament, freeInt, chess->tournamentsById)

struct chess_system_t {
    Map tournamentsById;
};

static bool isValidLocation(const char *location) {
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

ChessSystem chessCreate() {
    ChessSystem result = malloc(sizeof(*result));
    RETURN_NULL_IF_NULL(result);
    result->tournamentsById = mapCreate((copyMapDataElements) &copyTournament, (copyMapKeyElements) &copyInt,
                                        (freeMapDataElements) &freeTournament, (freeMapKeyElements) &freeInt,
                                        (compareMapKeyElements) &compareInt);
    if (result->tournamentsById == NULL) {
        free(result);
        return NULL;
    }
    return result;
}

void chessDestroy(ChessSystem chess) {
    if (chess == NULL) {
        return;
    }
    mapDestroy(chess->tournamentsById);
    free(chess);
}

ChessResult chessRemoveTournament(ChessSystem chess, int tournament_id) {
    if (chess == NULL) {
        return CHESS_NULL_ARGUMENT;
    }
    if (tournament_id <= 0) {
        return CHESS_INVALID_ID;
    }
    switch (mapRemove(chess->tournamentsById, &tournament_id)) {
        case MAP_ITEM_DOES_NOT_EXIST:
            return CHESS_TOURNAMENT_NOT_EXIST;
        case MAP_SUCCESS:
            return CHESS_SUCCESS;
        default:
            assert(false);
    }
    return CHESS_SUCCESS;
}

ChessResult chessEndTournament(ChessSystem chess, int tournament_id) {
    if (chess == NULL) {
        return CHESS_NULL_ARGUMENT;
    }
    if (tournament_id <= 0) {
        return CHESS_INVALID_ID;
    }

    Tournament tournament = mapGet(chess->tournamentsById, &tournament_id);
    return tournament == NULL ? CHESS_TOURNAMENT_NOT_EXIST : endTournament(tournament);
}

ChessResult chessRemovePlayer(ChessSystem chess, int player_id) {
    if (chess == NULL) {
        return CHESS_NULL_ARGUMENT;
    }
    if (player_id <= 0) {
        return CHESS_INVALID_ID;
    }
    bool was_removed = false;
    FOREACH_TOURNAMENT {
        was_removed = was_removed || tournamentRemovePlayer(tournament, player_id);
    }
    return was_removed ? CHESS_SUCCESS : CHESS_PLAYER_NOT_EXIST;
}

ChessResult chessAddTournament(ChessSystem chess, int tournament_id,
                               int max_games_per_player, const char *tournament_location) {
    if (chess == NULL || tournament_location == NULL) {
        return CHESS_NULL_ARGUMENT;
    }
    if (tournament_id <= 0) {
        return CHESS_INVALID_ID;
    }
    if (mapContains(chess->tournamentsById, &tournament_id)) {
        return CHESS_TOURNAMENT_ALREADY_EXISTS;
    }
    if (!isValidLocation(tournament_location)) {
        return CHESS_INVALID_LOCATION;
    }
    if (max_games_per_player <= 0) {
        return CHESS_INVALID_MAX_GAMES;
    }
    Tournament tournament = tournamentCreate(max_games_per_player, tournament_location);
    if (tournament == NULL) {
        return CHESS_OUT_OF_MEMORY;
    }
    ChessResult result =
            convertResults(mapPut(chess->tournamentsById, (MapKeyElement) &tournament_id, (MapDataElement) tournament));
    freeTournament(tournament);
    return result;
}

ChessResult chessAddGame(ChessSystem chess, int tournament_id, int first_player,
                         int second_player, Winner winner, int play_time) {
    switch (winner) {
        case FIRST_PLAYER:
        case SECOND_PLAYER:
        case DRAW:
            break;
        default:
            assert(false);
    }
    if (chess == NULL) {
        return CHESS_NULL_ARGUMENT;
    }
    if (tournament_id <= 0 || second_player == first_player || second_player <= 0 || first_player <= 0) {
        return CHESS_INVALID_ID;
    }
    if (play_time < 0) {
        return CHESS_INVALID_PLAY_TIME;
    }

    Tournament tournament = mapGet(chess->tournamentsById, &tournament_id);
    return tournament == NULL
           ? CHESS_TOURNAMENT_NOT_EXIST
           : gameCreate(tournament, first_player, second_player, winner, play_time);
}

double chessCalculateAveragePlayTime(ChessSystem chess, int player_id, ChessResult *chess_result) {
    ASSERT_NOT_NULL(chess_result);
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
    FOREACH_TOURNAMENT {
        struct play_stats_t player_stats = tournamentGetPlayerStats(tournament, player_id, chess_result);
        if (*chess_result != CHESS_SUCCESS)
            return 0;

        count += player_stats.num_wins + player_stats.num_losses + player_stats.num_draws;
        total_play_time += player_stats.total_play_time;
    }
    if (count == 0) {
        *chess_result = CHESS_PLAYER_NOT_EXIST;
        return 0;
    }
    *chess_result = CHESS_SUCCESS;
    return total_play_time / count;
}

ChessResult chessSavePlayersLevels(ChessSystem chess, FILE *file) {
    Map playersStats = mapCreate((copyMapDataElements) &copyStatsFunc, (copyMapKeyElements) &copyInt,
                                 (freeMapDataElements) &freeStatsFunc, (freeMapKeyElements) &freeInt,
                                 (compareMapKeyElements) &compareInt); //key:player_id, data:player stats
    if (playersStats == NULL)
        return CHESS_OUT_OF_MEMORY;
    FOREACH_TOURNAMENT {
        RETURN_IF_NOT_SUCCESS(tournamentUpdatePlayerStats(tournament, playersStats));
    }
    FOREACH_PLAYER_STATS(playersStats) {
        double level =
                (double) (6 * player_stats->num_wins - 10 * player_stats->num_losses + 2 * player_stats->num_draws) /
                (player_stats->num_draws + player_stats->num_losses + player_stats->num_wins);
        if (fprintf(file, "%d %2f\n", *player_id, level) < 0) {
            mapDestroy(playersStats);
            return CHESS_SAVE_FAILURE;
        }
    }
    mapDestroy(playersStats);
    return CHESS_SUCCESS;
}

ChessResult chessSaveTournamentStatistics(ChessSystem chess, char *path_file) {
    FILE *file = fopen(path_file, "w");
    bool tournament_has_ended = false;
    FOREACH_TOURNAMENT {
        if (tournamentHasEnded(tournament) == false) {
            continue;
        }
        tournament_has_ended = true;
        int tmp;
        PRINT_INT_TO_FILE(getWinner, tournament, tmp, file);
        PRINT_INT_TO_FILE(longestGameTime, tournament, tmp, file);
        PRINT_INT_TO_FILE(averageGameTime, tournament, tmp, file);
        const char *location = getLocation(tournament);
        ASSERT_NOT_NULL(location);
        fprintf(file, "%s\n", location);
        PRINT_INT_TO_FILE(getNumberOfPlayers, tournament, tmp, file);
        PRINT_INT_TO_FILE(getNumberOfGames, tournament, tmp, file);
    }
    return tournament_has_ended ? CHESS_SUCCESS : CHESS_NO_TOURNAMENTS_ENDED;
}
