#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "map.h"
#include <assert.h>
#include "chessSystem.h"
#include "tournaments.h"

#define MIN_LOCATION_LEN 3


#define CHECK_NULL(args) if ((args) == NULL) return NULL
#define NULL_ASSERT(args) assert(args != NULL)

typedef int TournamentId;

typedef struct chess_system_t {
    Map tournamentsById;
} *ChessSystem;

Tournament tournamentCreate(int max_games_per_player, const char *location);

TournamentId *copy_tournament_id(TournamentId *id) {
    TournamentId *result = malloc(sizeof(*result));
    CHECK_NULL(result);
    *result = *id;
    return result;
}

void free_tournament_id(TournamentId *id) {
    NULL_ASSERT(id);
    free(id);
}

int compare_tournament_id(TournamentId *id1, TournamentId *id2) {
    NULL_ASSERT(id1);
    NULL_ASSERT(id2);
    return *id1 - *id2;
}

bool locationIsValid(const char *location) {
    if (location == NULL || strlen(location) < MIN_LOCATION_LEN) {
        return false;
    }
    if (!(location[0] >= 'A' && location[0] <= 'Z')) {
        return false;
    }
    for (int i = 1; i < strlen(location) - 1; ++i) {
        if (location[i] == ' ' || (location[i] >= 'a' && location[i] <= 'z')) {
            continue;
        }
        return false;
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
    result->tournamentsById = mapCreate(&copy_tournament, &copy_tournament_id, &free_tournament, &free_tournament_id,
                                        &compare_tournament_id);
    CHECK_NULL(result->tournamentsById);
    return result;
}

void chessDestroy(ChessSystem chess) {
    if (chess == NULL) {
        return;
    }
    MAP_FOREACH(Map, iterator, chess->tournamentsById) {
        mapDestroy(chess->tournamentsById);
    }
    free(chess);
}

ChessResult chessRemoveTournament(ChessSystem chess, int tournament_id) {
    CHECK_NULL(chess);
    {
        return CHESS_NULL_ARGUMENT;
    }
    if (tournament_id <= 0) {
        return CHESS_INVALID_ID;
    }
    MAP_FOREACH(Map, iterator, chess->tournamentsById) {
        if (mapContains(chess->tournamentsById, (MapKeyElement)tournament_id)){
            mapDestroy(chess->tournamentsById);
            return CHESS_SUCCESS;
        }
        return CHESS_TOURNAMENT_NOT_EXIST;
    }
}
ChessResult chessEndTournament (ChessSystem chess, int tournament_id) {
    CHECK_NULL(chess);
    {
        return CHESS_NULL_ARGUMENT;
    }
    if (tournament_id <= 0) {
        return CHESS_INVALID_ID;
    }
    MAP_FOREACH(Map, iterator, chess->tournamentsById) {
        //todo: calculate the tournament winner.
        if (mapContains(chess->tournamentsById, (MapKeyElement) tournament_id)) {
            mapDestroy(chess->tournamentsById);
            return CHESS_SUCCESS;
        }
        return CHESS_TOURNAMENT_NOT_EXIST;
    }
}
ChessResult chessRemovePlayer(ChessSystem chess, int player_id){

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
    if (mapContains(chess->tournamentById, (MapKeyElement) &tournament_id) == true) {
        return CHESS_TOURNAMENT_ALREADY_EXISTS;
    }
    if (max_games_per_player <= 0) {
        return CHESS_INVALID_MAX_GAMES;
    }
    Tournament tournament = tournamentCreate(max_games_per_player, tournament_location);
    return tournament == NULL
           ? CHESS_OUT_OF_MEMORY
           : convertResults(
                    mapPut(chess->tournamentById, (MapKeyElement) &tournament_id, (MapDataElement) tournament));
}

ChessResult chessAddGame(ChessSystem chess, int tournament_id, int first_player,
                         int second_player, Winner winner, int play_time) {
    if (chess == NULL) {
        return CHESS_NULL_ARGUMENT;
    }
    if (tournament_id <= 0) {
        return CHESS_INVALID_ID;
    }
    if (mapContains(chess->tournamentById, (MapKeyElement) &tournament_id) == false) {
        return CHESS_TOURNAMENT_NOT_EXIST;
    }
    if ()
}

