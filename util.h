#ifndef CHESS_UTIL_H
#define CHESS_UTIL_H

#include "chessSystem.h"
#include "map.h"

#define RETURN_NULL_IF_NULL(args) if ((args) == NULL) return NULL

#define ASSERT_NOT_NULL(args) assert(args != NULL)

#define INVALID_ID 0

#define RETURN_IF_NOT_SUCCESS(result) do { ChessResult temp = (result); \
                                        if (temp != CHESS_SUCCESS) return temp; } while(0)

typedef struct play_stats_t {
    int num_wins;
    int num_losses;
    int num_draws;
    double total_play_time;
} *PlayerStats;

int *copyInt(const int *id);

void freeInt(int *id);

int compareInt(const int *id1, const int *id2);

void freeStatsFunc(PlayerStats stats);

PlayerStats copyStatsFunc(PlayerStats stats);

ChessResult convertResults(MapResult result);

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
