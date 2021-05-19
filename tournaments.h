#ifndef CHESS_TOURNAMENTS_H
#define CHESS_TOURNAMENTS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "map.h"
#include "util.h"

typedef struct tournament_t *Tournament;

// also checks if game already exists.
ChessResult gameCreate(Tournament tournament, int first_player, int second_player, Winner winner, int play_time);

bool tournamentHasEnded(Tournament tournament);

bool tournamentRemovePlayer(Tournament tournament, int player_id);

Tournament tournamentCreate(int max_games_per_player, const char *location);

PlayerStats tournamentGetPlayerStats(Tournament tournament, int player_id);

ChessResult tournamentUpdatePlayerStats(Tournament tournament, Map playersStatsById);

// Returns the winner's player ID, or -1 if there are no games.
int calculateTournamentWinner(Tournament tournament);

void updateStatisticsForTournament(Tournament tournament);
int getWinner(Tournament tournament);//return INVALID_ID if fails

double longestGameTime(Tournament tournament);//return INVALID_ID if fails

double averageGameTime(Tournament tournament);//return INVALID_ID if fails

const char *getLocation(Tournament tournament);//return NULL if fails

int getNumberOfPlayers(Tournament tournament);//return INVALID_ID if fails

int getNumberOfGames(Tournament tournament);//return INVALID_ID if fails

Tournament copyTournament(Tournament tournament);

void freeTournament(Tournament tournament);

bool endTournament(Tournament tournament); // Return false if already ended.

#endif //CHESS_TOURNAMENTS_H
