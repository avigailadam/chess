#include "tournaments.h"

#define REMOVE_PLAYER(player_to_remove, winner_player)  (player_to_remove) = INVALID_ID;\
                                                        gameData->winner = winner_player
#define FOREACH_GAME MAP_FOREACH_VALUE(GameKey, gameId, GameData, gameData, free_game_key, tournament->gameByBothPlayersId)

#define FIRST_PLACE(scorer) \
do {                        \
MapKeyElement best_player = mapGetFirst(scores); \
MAP_FOREACH(int*, player, scores) {       \
if (scorer(scores, player) >= scorer(scores, best_player)) {\
best_player = player;\
}\
}\
MAP_FOREACH(int*, player, scores) {\
if (scorer(scores, best_player)>scorer(scores, player)) {\
mapRemove(scores, player);\
}\
}\
if (mapGetSize(scores) == 1) {\
int winner = (int) mapGetFirst(scores);\
mapDestroy(scores);\
mapDestroy(players_to_stats);\
return winner;\
}\
}while(0)
typedef struct tournament_t {
    Map gameByBothPlayersId;
    int max_games_per_player;
    const char *location;
    int winner;
} *Tournament;

typedef struct game_key_t {
    int player_1_id;
    int player_2_id;
} *GameKey;


typedef struct game_t {
    Winner winner;
    int duration;
} *GameData;

void addEmptyStatsIfNotExists(Map stats, int player_id) {
    if (mapContains(stats, (&player_id))) {
        return;
    }
    struct play_stats_t empty;
    empty.total_play_time = 0;
    empty.num_draws = 0;
    empty.num_losses = 0;
    empty.num_wins = 0;
    mapPut(stats, &player_id, &empty);
}

GameKey copy_game_key(GameKey game_key) {
    ASSERT_NOT_NULL(game_key);
    GameKey new_key = malloc(sizeof(*new_key));
    RETURN_NULL_IF_NULL(new_key);
    new_key->player_1_id = game_key->player_1_id;
    new_key->player_2_id = game_key->player_2_id;
    return new_key;
}

GameData copy_game_data(GameData game_data) {
    ASSERT_NOT_NULL(game_data);
    GameData new_data = malloc(sizeof(*new_data));
    RETURN_NULL_IF_NULL(new_data);
    new_data->duration = game_data->duration;
    new_data->winner = game_data->winner;
    return new_data;
}

void free_game_data(GameData game_data) {
    free(game_data);
}

void free_game_key(GameKey game_key) {
    free(game_key);
}

int compare_game_key(GameKey game_key_1, GameKey game_key_2) {
    int player_1_diff = game_key_1->player_1_id - game_key_2->player_1_id;
    int player_2_diff = game_key_1->player_2_id - game_key_2->player_2_id;
    return player_1_diff + player_2_diff;
}

Tournament tournamentCreate(int max_games_per_player, const char *location) {
    Tournament tournament = malloc(sizeof(*tournament));
    if (tournament == NULL) {
        return NULL;
    }
    tournament->gameByBothPlayersId = mapCreate((copyMapDataElements) &copy_game_data,
                                                (copyMapKeyElements) &copy_game_key,
                                                (freeMapDataElements) &free_game_data,
                                                (freeMapKeyElements) &free_game_key,
                                                (compareMapKeyElements) &compare_game_key);
    tournament->location = location;
    tournament->max_games_per_player = max_games_per_player;
    tournament->winner = INVALID_ID;
    return tournament;
}

static int getNumOfWinsAux(Map scores, int *player) {
    assert(scores != NULL);
    PlayerStats stats = mapGet(scores, &player);
    ASSERT_NOT_NULL(stats);
    int result = stats->num_wins;
    freeStatsFunc(stats);
    return result;
}

static int getNumOfLosses(Map scores, int *player) {
    assert(scores != NULL);
    PlayerStats stats = mapGet(scores, &player);
    ASSERT_NOT_NULL(stats);
    int result = stats->num_losses;
    freeStatsFunc(stats);
    return result;
}

void addPlayerStats(Map stats_by_players, GameData this_game_data, int player_id_winner, int player_id_loser) {
    assert(stats_by_players != NULL);
    PlayerStats stats_winner = mapGet(stats_by_players, &player_id_winner);
    ASSERT_NOT_NULL(stats_winner);
    stats_winner->num_wins++;
    mapPut(stats_by_players, &player_id_winner, stats_winner);
    freeStatsFunc(stats_winner);
    PlayerStats stats_loser = mapGet(stats_by_players, &player_id_loser);
    ASSERT_NOT_NULL(stats_loser);
    stats_loser->num_losses++;
    mapPut(stats_by_players, &player_id_loser, stats_loser);
    freeStatsFunc(stats_loser);
}

#define UPDATE_DRAW(player_id) do {          \
 stats = mapGet(result, &gameId->player_id); \
 ASSERT_NOT_NULL(stats);                     \
 stats->num_draws++; } while(0)


static Map getStatsByPlayer(Tournament tournament) {
    Map result = mapCreate((copyMapDataElements) &copyStatsFunc, (copyMapKeyElements) &copyInt,
                           (freeMapDataElements) &freeStatsFunc, (freeMapKeyElements) &freeInt,
                           (compareMapKeyElements) &compareInt);
    ASSERT_NOT_NULL(result);

    FOREACH_GAME {
        addEmptyStatsIfNotExists(result, gameId->player_1_id);
        addEmptyStatsIfNotExists(result, gameId->player_2_id);
        PlayerStats stats;
        switch (gameData->winner) {
            case FIRST_PLAYER:
                addPlayerStats(result, gameData, gameId->player_1_id, gameId->player_2_id);
                break;
            case SECOND_PLAYER:
                addPlayerStats(result, gameData, gameId->player_2_id, gameId->player_1_id);
                break;
            case DRAW:
                UPDATE_DRAW(player_1_id);
                UPDATE_DRAW(player_2_id);
            default:
                assert(0);
        }
        if (gameData->winner == FIRST_PLAYER) {
            addPlayerStats(result, gameData, gameId->player_1_id, FIRST_PLAYER);
            continue;
        }
        if (gameData->winner == SECOND_PLAYER) {
            addPlayerStats(result, gameData, gameId->player_2_id, SECOND_PLAYER);
            continue;
        }
        addPlayerStats(result, gameData, gameId->player_2_id, DRAW);
    }
    return result;
}

int calculateTournamentWinner(Tournament tournament) {
    Map players_to_stats = getStatsByPlayer(tournament);
    if (mapGetSize(players_to_stats) == 0) {
        return INVALID_ID;
    }
    Map scores;
    scores = mapCreate((copyMapDataElements) &copyInt, (copyMapKeyElements) &copyInt, (freeMapDataElements) &freeInt,
                       (freeMapKeyElements) &freeInt, (compareMapKeyElements) &compareInt);
    RETURN_NULL_IF_NULL(scores);
    MAP_FOREACH(int*, player, players_to_stats) {
        PlayerStats playerState = tournamentGetPlayerStats(tournament, *player);
        int score = (playerState->num_draws) * 1 + (playerState->num_wins) * 2;
        mapPut(scores, player, &score);
        freeStatsFunc(playerState);
    }

    FIRST_PLACE(mapGet);
    MapKeyElement least_losses = mapGetFirst(scores);
    MAP_FOREACH(int*, player, players_to_stats) {
        if (getNumOfLosses(scores, least_losses) >= getNumOfLosses(scores, player)) {
            least_losses = player;
        }
    }
    MAP_FOREACH(int*, player, players_to_stats) {
        if (getNumOfLosses(scores, player) > getNumOfLosses(scores, least_losses)) {
            mapRemove(scores, player);
        }
    }
    if (mapGetSize(scores) == 1) {
        int winner = (int) mapGetFirst(scores);
        mapDestroy(scores);
        mapDestroy(players_to_stats);
        return winner;
    }
    FIRST_PLACE(getNumOfWinsAux);
    int winner = (int) mapGetFirst(scores);
    mapDestroy(scores);
    mapDestroy(players_to_stats);
    return winner;
}

ChessResult gameCreate(Tournament tournament, int first_player, int second_player, Winner winner, int play_time) {
    ASSERT_NOT_NULL(tournament);
    assert(first_player >= 0);
    assert(second_player >= 0);
    assert(play_time >= 0);
    struct game_key_t key;
    key.player_1_id = first_player;
    key.player_2_id = second_player;
    if (mapContains(tournament->gameByBothPlayersId, &key)) {
        return CHESS_GAME_ALREADY_EXISTS;
    }
    struct game_t data;
    data.winner = winner;
    data.duration = play_time;
    return convertResults(mapPut(tournament->gameByBothPlayersId, &key, &data));
}

bool tournamentHasEnded(Tournament tournament) {
    ASSERT_NOT_NULL(tournament);
    return tournament->winner > 0;
}

bool tournamentRemovePlayer(Tournament tournament, int player_id) {
    ASSERT_NOT_NULL(tournament);
    bool playerExists = false;
    FOREACH_GAME {
        bool isPlayer1 = compareInt(&(gameId->player_1_id), &player_id) == 0;
        bool isPlayer2 = compareInt(&(gameId->player_2_id), &player_id) == 0;
        if (!isPlayer1 && !isPlayer2) {
            continue;
        }
        playerExists = true;
        assert(!(isPlayer1 && isPlayer2));
        if (isPlayer1) {
            REMOVE_PLAYER(gameId->player_1_id, SECOND_PLAYER);
        }
        if (isPlayer2) {
            REMOVE_PLAYER(gameId->player_2_id, FIRST_PLAYER);
        }
    }
    return playerExists;
}

Tournament copyTournament(Tournament tournament) {
    Tournament new = tournamentCreate(tournament->max_games_per_player, tournament->location);
    RETURN_NULL_IF_NULL(new);
    new->gameByBothPlayersId = mapCopy(tournament->gameByBothPlayersId);
    RETURN_NULL_IF_NULL(new->gameByBothPlayersId);
    new->winner = tournament->winner;
    return new;
}

void freeTournament(Tournament tournament) {
    ASSERT_NOT_NULL(tournament);
    mapDestroy(tournament->gameByBothPlayersId);
    free(tournament);
}

const char *getLocation(Tournament tournament) {
    ASSERT_NOT_NULL(tournament);
    return tournament->location;
}

int getNumberOfGames(Tournament tournament) {
    ASSERT_NOT_NULL(tournament);
    return mapGetSize(tournament->gameByBothPlayersId);
}

int getNumberOfPlayers(Tournament tournament) {
    ASSERT_NOT_NULL(tournament);
    return mapGetSize(getStatsByPlayer(tournament));
}

double averageGameTime(Tournament tournament) {
    ASSERT_NOT_NULL(tournament);
    int count = 0;
    double sum = 0;
    FOREACH_GAME {
        sum += gameData->duration;
        count++;
    }
    assert(count > 0);
    return sum / count;
}

double longestGameTime(Tournament tournament) {
    ASSERT_NOT_NULL(tournament);
    int max = -1;
    FOREACH_GAME {
        int time = gameData->duration;
        assert(time > 0);
        max = max > time ? time : max;
    }
    assert(max > 0);
    return max;
}

int getWinner(Tournament tournament) {
    ASSERT_NOT_NULL(tournament);
    assert(tournament->winner != INVALID_ID);
    return tournament->winner;
}

bool endTournament(Tournament tournament) {
    ASSERT_NOT_NULL(tournament);
    if (tournament->winner != INVALID_ID)
        return false;
    tournament->winner = calculateTournamentWinner(tournament);
    return true;
}

ChessResult tournamentUpdatePlayerStats(Tournament tournament, Map playersStatsById) {
    ASSERT_NOT_NULL(tournament);
    ASSERT_NOT_NULL(playersStatsById);
    assert(tournamentHasEnded(tournament));
    Map stats = getStatsByPlayer(tournament);
    if (stats == NULL) {
        return CHESS_OUT_OF_MEMORY;
    }
    FOREACH_PLAYER_STATS(stats) {
        addEmptyStatsIfNotExists(playersStatsById, *player_id);
        PlayerStats existingStats = mapGet(playersStatsById, player_id);
        existingStats->num_wins += player_stats->num_wins;
        existingStats->num_losses += player_stats->num_losses;
        existingStats->num_draws += player_stats->num_draws;
        existingStats->total_play_time += player_stats->total_play_time;
    }
    mapDestroy(stats);
    return CHESS_SUCCESS;
}

PlayerStats tournamentGetPlayerStats(Tournament tournament, int player_id) {
    PlayerStats result = mapGet(getStatsByPlayer(tournament), &player_id);
    ASSERT_NOT_NULL(result);
    return result;
}
