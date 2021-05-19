#ifndef CHESS_TOURNAMENTS_H
#define CHESS_TOURNAMENTS_H

#include "util.h"

typedef struct tournament_t *Tournament;

bool doesGameExist(int first_player, int second_player);

ChessResult gameCreate(int first_player, int second_player, Winner winner, int play_time);

bool tournamentHasEnded(Tournament);

bool tournamentRemovePlayer(Tournament tournament, int player_id);

Tournament tournamentCreate(int max_games_per_player, const char *location);

PlayerStats tournamentGetPlayerStats(Tournament tournament, int player_id);

ChessResult tournamentUpdatePlayerStats(Tournament tournament, Map playersStatsById);

int calculateTournamentWinner(Map players_by_id, Tournament tournament);

void updateStatisticsForTournament(Tournament tournament);

int getNumOfLosses(int player);

int getNumOfWins(int player);

int getWinner(Tournament tournament);//return -1 if fails

double longestGameTime(Tournament tournament);//return -1 if fails

double averageGameTime(Tournament tournament);//return -1 if fails

const char *getLocation(Tournament tournament);//return NULL if fails

int getNumberOfPlayers(Tournament tournament);//return -1 if fails

int getNumberOfGames(Tournament tournament);//return -1 if fails

Tournament copyTournament(Tournament tournament);

void freeTournament(Tournament tournament);

#endif //CHESS_TOURNAMENTS_H
