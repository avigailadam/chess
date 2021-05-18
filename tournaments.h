#ifndef CHESS_TOURNAMENTS_H
#define CHESS_TOURNAMENTS_H


typedef struct tournament_t *Tournament;


Tournament tournamentCreate(int max_games_per_player, const char *location);
#endif //CHESS_TOURNAMENTS_H

 int calculateTournamentWinner(Tournament tournament);
void updateStatisticsForTournament(Tournament tournament);