#ifndef CHESS_TOURNAMENTS_H
#define CHESS_TOURNAMENTS_H

typedef struct play_stats_t {
    int num_wins;
    int num_losses;
    int num_draws;
    double total_play_time;
} *PlayerStats;

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

#endif //CHESS_TOURNAMENTS_H
