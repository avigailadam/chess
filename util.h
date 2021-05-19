#include "chessSystem.h"

#define RETURN_NULL_IF_NULL(args) if ((args) == NULL) return NULL
#define ASSERT_NOT_NULL(args) assert(args != NULL)

typedef struct play_stats_t {
    int num_wins;
    int num_losses;
    int num_draws;
    double total_play_time;
} *PlayerStats;

int *copyInt(const int *id) {
    RETURN_NULL_IF_NULL(id);
    int *new_id = malloc(sizeof(*new_id));
    return new_id;
}

void freeInt(int *id) {
    ASSERT_NOT_NULL(id);
    free(id);
}

int compareInt(const int *id1, const int *id2) {
    ASSERT_NOT_NULL(id1);
    ASSERT_NOT_NULL(id2);
    return *id1 - *id2;
}

void freeStatsFunc(PlayerStats stats) {
    free(stats);
}

PlayerStats copyStatsFunc(PlayerStats stats) {
    RETURN_NULL_IF_NULL(stats);
    PlayerStats new = malloc(sizeof(*new));
    RETURN_NULL_IF_NULL(new);
    new->num_wins = stats->num_wins;
    new->num_losses = stats->num_losses;
    new->num_draws = stats->num_draws;
    new->total_play_time = stats->total_play_time;
    return new;
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

#ifndef CHESS_UTIL_H
#define CHESS_UTIL_H

#endif //CHESS_UTIL_H
