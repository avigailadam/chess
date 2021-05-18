#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "map.h"
#include "chessSystem.h"
#include "tournaments.h"

#define CHECK_NULL(args) if ((args) == NULL) return NULL

typedef int TournamentId;

typedef struct chess_system_t {
    Map tournamentById;
} *ChessSystem;

TournamentId* copy_tournament_id(TournamentId* id){
   TournamentId * result = malloc(sizeof(*result));
   CHECK_NULL(result);
   *result = *id;
   return result;
}

ChessSystem chessCreate() {
    ChessSystem result = malloc(sizeof(*result));
    CHECK_NULL(result);
    result->tournamentById = mapCreate(&copy_tournament, &copy_tournament_id, &free_tournament, &free_tournament_id,
                                       &compare_tournament_id);
    CHECK_NULL(result->tournamentById);
    return result;
}