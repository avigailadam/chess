#ifndef CHESS_UTIL_H
#define CHESS_UTIL_H

#include "assert.h"
#include "chessSystem.h"
#include "map.h"
#include <stdlib.h>

#define INVALID_ID -1

#define RETURN_NULL_IF_NULL(args) if ((args) == NULL) return NULL

#define ASSERT_NOT_NULL(args) assert(args != NULL)

#define INVALID_ID -1

#define RETURN_IF_NOT_SUCCESS(result) do { ChessResult temp = (result); if (temp != CHESS_SUCCESS) return temp; } while(0)

typedef struct play_stats_t {
    int num_wins;
    int num_losses;
    int num_draws;
    double total_play_time;
} *PlayerStats;

int *copyInt(const int *id) {
    ASSERT_NOT_NULL(id);
    int *new_id = malloc(sizeof(*new_id));
    *new_id = *id;
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

#define MAP_FOREACH_VALUE(key_type, key_iter, value_type, value_iter, free_key_iter, map) \
    value_type value_iter = NULL;                                                         \
    key_type key_iter = (key_type) mapGetFirst(map);                                      \
    value_iter = (value_type)(key_iter ? mapGet(map, key_iter) : value_iter);             \
    for( ;                                                                                \
        key_iter && (ASSERT_NOT_NULL(value_iter), true);                                  \
        free_key_iter(key_iter),                                                          \
        key_iter = mapGetNext(map),                                                       \
        value_iter = key_iter ? (value_type)mapGet(map, key_iter) : value_iter)

#define FOREACH_PLAYER_STATS(map) MAP_FOREACH_VALUE(int*, player_id, PlayerStats, player_stats, freeInt, map)

#endif //CHESS_UTIL_H
