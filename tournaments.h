#ifndef CHESS_TOURNAMENTS_H
#define CHESS_TOURNAMENTS_H

#include "games.h"

typedef struct tournament_t *Tournament;


Tournament tournamentCreate(int max_games_per_player, const char *location);
#endif //CHESS_TOURNAMENTS_H
