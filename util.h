#define CHECK_NULL(args) if ((args) == NULL) return NULL
#define NULL_ASSERT(args) assert(args != NULL)

typedef struct play_stats_t {
    int num_wins;
    int num_losses;
    int num_draws;
    double total_play_time;
} *PlayerStats;

int *copyInt(int *id) {
    CHECK_NULL(id);
    int *new_id = malloc(sizeof(*new_id));
    return new_id;
}

void freeInt(int *id) {
    NULL_ASSERT(id);
    free(id);
}

int compareInt(const int *id1, const int *id2) {
    NULL_ASSERT(id1);
    NULL_ASSERT(id2);
    return *id1 - *id2;
}

void freeStatsFunc(PlayerStats stats) {
    free(stats);
}

PlayerStats copyStatsFunc(PlayerStats stats) {
    CHECK_NULL(stats);
    PlayerStats new = malloc(sizeof(*new));
    CHECK_NULL(new);
    new->num_wins = stats->num_wins;
    new->num_losses = stats->num_losses;
    new->num_draws = stats->num_draws;
    new->total_play_time = stats->total_play_time;
    return new;
}

#ifndef CHESS_UTIL_H
#define CHESS_UTIL_H

#endif //CHESS_UTIL_H
