#include "tournaments.h"

#define FOREACH_GAME MAP_FOREACH_VALUE(GameKey, gameId, GameData, gameData, free_game_key, tournament->gameByBothPlayersId)

#define FIRST_PLACE(scorer, result) \
    do {                            \
        int* temp = mapGetFirst(scores);\
        int best_player = *temp;    \
        free(temp);                            \
        MAP_FOREACH(int*, player, scores) {       \
            if (scorer(scores, player) >= scorer(scores, &best_player)) {\
                best_player = *player;\
            }                       \
            free(player);                            \
        }\
        MAP_FOREACH(int*, player, players_to_stats) {\
            if (scorer(scores, &best_player)>scorer(scores, player)) {\
                mapRemove(scores, player);\
            }                       \
            free(player);                        \
        }\
        if (mapGetSize(scores) == 1) {\
            int* winner= mapGetFirst(scores);\
            *result=*winner;        \
            free(winner);\
            mapDestroy(scores);\
            mapDestroy(players_to_stats);      \
            return CHESS_SUCCESS;   \
        }\
    } while(0)
struct tournament_t {
    Map gameByBothPlayersId;
    int max_games_per_player;
    const char *location;
    int winner;
    int participants;
};

typedef struct game_key_t {
    int player_1_id;
    int player_2_id;
} *GameKey;


typedef struct game_t {
    Winner winner;
    int duration;
} *GameData;

struct play_stats_t createEmptyStats();

MapResult addEmptyStatsIfNotExists(Map stats, int player_id) {
    ASSERT_NOT_NULL(stats);
    if (mapContains(stats, (&player_id))) {
        return MAP_SUCCESS;
    }
    struct play_stats_t empty = createEmptyStats();
    return mapPut(stats, &player_id, &empty);
}

struct play_stats_t createEmptyStats() {
    struct play_stats_t empty;
    empty.total_play_time = 0;
    empty.num_draws = 0;
    empty.num_losses = 0;
    empty.num_wins = 0;
    return empty;
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
    int first_compare = compareInt(&game_key_1->player_1_id, &game_key_2->player_1_id);
    if (first_compare != 0) {
        return first_compare;
    }
    return compareInt(&game_key_1->player_2_id, &game_key_2->player_2_id);
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
    if (tournament->gameByBothPlayersId == NULL) {
        free(tournament);
        return NULL;
    }
    tournament->location = location;
    tournament->max_games_per_player = max_games_per_player;
    tournament->winner = INVALID_ID;
    tournament->participants = 0;
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


static int mapGetAux(Map scores, int *player) {
    ASSERT_NOT_NULL(scores);
    int *result = mapGet(scores, player);
    ASSERT_NOT_NULL(result);
    return *result;
}

static int getNumOfLosses(Map scores, int *player) {
    assert(scores != NULL);
    PlayerStats stats = mapGet(scores, &player);
    ASSERT_NOT_NULL(stats);
    int result = stats->num_losses;
    freeStatsFunc(stats);
    return result;
}

static ChessResult addPlayerStats(Map stats_by_players, int player_id_winner, int player_id_loser) {
    assert(stats_by_players != NULL);
    PlayerStats stats_winner = mapGet(stats_by_players, &player_id_winner);
    ASSERT_NOT_NULL(stats_winner);
    stats_winner->num_wins++;
    RETURN_IF_NOT_SUCCESS(convertResults(mapPut(stats_by_players, &player_id_winner, stats_winner)));
    PlayerStats stats_loser = mapGet(stats_by_players, &player_id_loser);
    ASSERT_NOT_NULL(stats_loser);
    stats_loser->num_losses++;
    return convertResults(mapPut(stats_by_players, &player_id_loser, stats_loser));
}

#define UPDATE_DRAW(player_id) do {                  \
 PlayerStats stats=mapGet(stats_by_players,&gameId->player_id); \
ASSERT_NOT_NULL(stats);                             \
stats->num_draws++;} while(0)
#define UPDATE_WINNER(player_id_winner, player_id_loser) \
do {if (addPlayerStats(stats_by_players, player_id_winner, player_id_loser) != CHESS_SUCCESS) { \
  mapDestroy(stats_by_players); \
  return NULL; \
}} while(0)

// Returns NULL if allocation failed.
static Map getStatsByPlayer(Tournament tournament) {
    Map stats_by_players = mapCreate((copyMapDataElements) &copyStatsFunc, (copyMapKeyElements) &copyInt,
                                     (freeMapDataElements) &freeStatsFunc, (freeMapKeyElements) &freeInt,
                                     (compareMapKeyElements) &compareInt);
    RETURN_NULL_IF_NULL(stats_by_players);

    FOREACH_GAME {
        if (addEmptyStatsIfNotExists(stats_by_players, gameId->player_1_id) != MAP_SUCCESS ||
            addEmptyStatsIfNotExists(stats_by_players, gameId->player_2_id) != MAP_SUCCESS) {
            mapDestroy(stats_by_players);
            return NULL;
        }
        switch (gameData->winner) {
            case FIRST_PLAYER:
                UPDATE_WINNER(gameId->player_1_id, gameId->player_2_id);
                break;
            case SECOND_PLAYER:
                UPDATE_WINNER(gameId->player_2_id, gameId->player_1_id);
                break;
            case DRAW:
                UPDATE_DRAW(player_1_id);
                UPDATE_DRAW(player_2_id);
                break;
            default:
                assert(0);
        }
    }
    return stats_by_players;
}

static ChessResult calculateTournamentWinner(Tournament tournament, int *result) {
    ASSERT_NOT_NULL(result);
    Map players_to_stats = getStatsByPlayer(tournament);
    if (players_to_stats == NULL) {
        return CHESS_OUT_OF_MEMORY;
    }
    if (mapGetSize(tournament->gameByBothPlayersId) == 0) {
        return CHESS_NO_GAMES;
    }
    if (mapGetSize(players_to_stats) == 0) {
        return INVALID_ID;
    }
    Map scores;
    scores = mapCreate((copyMapDataElements) &copyInt, (copyMapKeyElements) &copyInt,
                       (freeMapDataElements) &freeInt,
                       (freeMapKeyElements) &freeInt, (compareMapKeyElements) &compareInt);
    if (scores == NULL) {
        return CHESS_OUT_OF_MEMORY;
    }
    MAP_FOREACH(int*, player, players_to_stats) {
        PlayerStats player_state = mapGet(players_to_stats, player);
        int score = (player_state->num_draws) * 1 + (player_state->num_wins) * 2;
        mapPut(scores, player, &score);
        free(player);
    }

    FIRST_PLACE(mapGetAux, result);
    MapKeyElement least_losses = mapGetFirst(scores);
    MAP_FOREACH(int*, player, players_to_stats) {
        if (getNumOfLosses(scores, least_losses) >= getNumOfLosses(scores, player)) {
            least_losses = player;
        }
        free(player);
    }
    MAP_FOREACH(int*, player, players_to_stats) {
        if (getNumOfLosses(scores, player) > getNumOfLosses(scores, least_losses)) {
            mapRemove(scores, player);
        }
        free(player);
    }
    if (mapGetSize(scores) == 1) {
        int *winner = mapGetFirst(scores);
        *result = *winner;
        mapDestroy(scores);
        mapDestroy(players_to_stats);
        return CHESS_SUCCESS;
    }
    FIRST_PLACE(getNumOfWinsAux, result);
    int *winner = mapGetFirst(scores);
    *result = *winner;
    mapDestroy(scores);
    mapDestroy(players_to_stats);
    return CHESS_SUCCESS;
}

ChessResult countGamesPerPlayer(Tournament tournament, int player_id, int *result) {
    ASSERT_NOT_NULL(result);
    *result = 0;
    Map stats = getStatsByPlayer(tournament);
    if (stats == NULL) {
        return CHESS_OUT_OF_MEMORY;
    }
    ASSERT_NOT_NULL(stats);
    PlayerStats statsForPlayer = mapGet(stats, &player_id);
    if (statsForPlayer != NULL) {
        *result = statsForPlayer->num_draws + statsForPlayer->num_losses + statsForPlayer->num_wins;
    }
    mapDestroy(stats);
    return CHESS_SUCCESS;
}

ChessResult getNewPlayers(Tournament tournament, int player_1_id, int player_2_id, int *result) {
    *result = 0;
    Map stats = getStatsByPlayer(tournament);
    if (stats == NULL) {
        return CHESS_OUT_OF_MEMORY;
    }
    if (mapContains(stats, &player_1_id) == false) {
        *result += 1;
    }
    if (mapContains(stats, &player_2_id) == false) {
        *result += 1;
    }
    mapDestroy(stats);
    return CHESS_SUCCESS;
}

ChessResult
gameCreate(Tournament tournament, int first_player, int second_player, Winner winner, int play_time) {
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
    int gamesPerPlayer1;
    int gamesPerPlayer2;
    if (countGamesPerPlayer(tournament, first_player, &gamesPerPlayer1) == CHESS_OUT_OF_MEMORY) {
        return CHESS_OUT_OF_MEMORY;
    }
    if (countGamesPerPlayer(tournament, second_player, &gamesPerPlayer2) == CHESS_OUT_OF_MEMORY) {
        return CHESS_OUT_OF_MEMORY;
    }
    if (gamesPerPlayer1 >= tournament->max_games_per_player ||
        gamesPerPlayer2 >= tournament->max_games_per_player) {
        assert(gamesPerPlayer1 == tournament->max_games_per_player ||
               gamesPerPlayer2 == tournament->max_games_per_player);
        return CHESS_EXCEEDED_GAMES;
    }

    struct game_t data;
    data.winner = winner;
    data.duration = play_time;
    int newPlayers;
    RETURN_IF_NOT_SUCCESS(getNewPlayers(tournament, first_player, second_player, &newPlayers));
    tournament->participants += newPlayers;
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
        assert(!isPlayer1 || !isPlayer2);
        assert(isPlayer1 || isPlayer2);
        *(isPlayer1 ? &gameId->player_1_id : &gameId->player_2_id) = INVALID_ID;
        if (tournamentHasEnded(tournament) == false) {
            gameData->winner = isPlayer1 ? SECOND_PLAYER : FIRST_PLAYER;
        }
    }
    return playerExists;
}

Tournament copyTournament(Tournament tournament) {
    Tournament new = tournamentCreate(tournament->max_games_per_player, tournament->location);
    RETURN_NULL_IF_NULL(new);
    mapDestroy(new->gameByBothPlayersId);
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
    return tournament->participants;
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
        max = max < time ? time : max;
    }
    assert(max > 0);
    return max;
}

int getWinner(Tournament tournament) {
    ASSERT_NOT_NULL(tournament);
    assert(tournament->winner != INVALID_ID);
    return tournament->winner;
}

ChessResult endTournament(Tournament tournament) {
    ASSERT_NOT_NULL(tournament);
    if (tournament->winner != INVALID_ID)
        return CHESS_TOURNAMENT_ENDED;
    int winner;
    RETURN_IF_NOT_SUCCESS(calculateTournamentWinner(tournament, &winner));
    if (winner == CHESS_NO_GAMES) {
        return CHESS_NO_GAMES;
    }
    tournament->winner = winner;
    return CHESS_SUCCESS;
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

struct play_stats_t tournamentGetPlayerStats(Tournament tournament, int player_id, ChessResult *chessResult) {
    ASSERT_NOT_NULL(chessResult);
    Map stats = getStatsByPlayer(tournament);
    if (stats == NULL) {
        *chessResult = CHESS_OUT_OF_MEMORY;
        return createEmptyStats();
    }
    PlayerStats playerStats = mapGet(stats, &player_id);
    *chessResult = CHESS_SUCCESS;
    struct play_stats_t results = playerStats == NULL ? createEmptyStats() : *playerStats;
    mapDestroy(stats);
    return results;
}
