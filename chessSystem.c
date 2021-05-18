#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "chessSystem.h"
#include "map.h"

typedef struct game_data_t {
    int player_1_id;
    int player_2_id;
    Winner winner;
    int duration;
} *GameData;

typedef struct game_t {
    GameData data;
    struct game_t *next;
} *Game;

typedef struct tournament_data_t {
    Game head;
    int id;
    int venue;
    int winner;
} *TournamentData;

typedef struct tournament_t {
    TournamentData data;
    struct tournament_t *next;
} *Tournament;

typedef struct chess_system_t {
    Tournament head;
} *ChessSystem;

ChessSystem chessCreate(){


}