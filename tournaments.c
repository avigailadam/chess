#include "tournaments.h"

#define FOREACH_GAME MAP_FOREACH_VALUE(GameKey, gameId, GameData, gameData, free_game_key, tournament->gameByBothPlayersId)

#define FILTER_FIRST(scorer) \
    do {                            \
        int* temp = mapGetFirst(scores);\
        int best_player = *temp;    \
        freeInt(temp);                            \
        MAP_FOREACH(int*, player, scores) {       \
            if (scorer(scores, player) >= scorer(scores, &best_player)) {\
                best_player = *player;\
            }                       \
            freeInt(player);                            \
        }\
        MAP_FOREACH(int*, player, players_to_stats) {                    \
            if (mapContains(scores, player) == false) {                  \
              freeInt(player);               \
              continue;        \
            }                \
            if (scorer(scores, &best_player) > scorer(scores, player)) {\
                mapRemove(scores, player);\
            }                       \
            freeInt(player);                        \
        }\
        if (mapGetSize(scores) == 1) {\
            int* winner= mapGetFirst(scores);\
            *result=*winner;        \
            freeInt(winner);\
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

static Map removeInvalidPlayers(Map map);

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
    if (game_key_1->player_1_id == game_key_2->player_2_id && game_key_1->player_2_id == game_key_2->player_1_id) {
        return 0;
    }
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
    PlayerStats stats = mapGet(scores, player);
    ASSERT_NOT_NULL(stats);
    return stats->num_wins;
}


static int mapGetAux(Map scores, int *player) {
    ASSERT_NOT_NULL(scores);
    int *result = mapGet(scores, player);
    ASSERT_NOT_NULL(result);
    return *result;
}

static int getNumOfLosses(Map scores, int *player) {
    assert(scores != NULL);
    PlayerStats stats = mapGet(scores, player);
    ASSERT_NOT_NULL(stats);
    return stats->num_losses;
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

#define UPDATE_ON_DRAW(player_id) do {                  \
 PlayerStats stats=mapGet(stats_by_players,&gameId->player_id); \
ASSERT_NOT_NULL(stats);                             \
stats->num_draws++;} while(0)
#define UPDATE_ON_WIN(player_id_winner, player_id_loser) \
do {if (addPlayerStats(stats_by_players, player_id_winner, player_id_loser) != CHESS_SUCCESS) { \
  mapDestroy(stats_by_players); \
  return NULL; \
}} while(0)

// Returns NULL if allocation failed.
// Returns a map from player ID to that player's stats.
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
                UPDATE_ON_WIN(gameId->player_1_id, gameId->player_2_id);
                break;
            case SECOND_PLAYER:
                UPDATE_ON_WIN(gameId->player_2_id, gameId->player_1_id);
                break;
            case DRAW:
                UPDATE_ON_DRAW(player_1_id);
                UPDATE_ON_DRAW(player_2_id);
                break;
            default:
                assert(0);
        }
        PlayerStats stat1 = mapGet(stats_by_players, &gameId->player_1_id);
        ASSERT_NOT_NULL(stat1);
        PlayerStats stat2 = mapGet(stats_by_players, &gameId->player_2_id);
        ASSERT_NOT_NULL(stat2);
        stat1->total_play_time += gameData->duration;
        stat2->total_play_time += gameData->duration;
    }
    return removeInvalidPlayers(stats_by_players);
}

static Map removeInvalidPlayers(Map player_stats) {
    ASSERT_NOT_NULL(player_stats);
    int size = mapGetSize(player_stats);
    int *invalid_ids = malloc(size * sizeof(int));
    int index = 0;
    RETURN_NULL_IF_NULL(invalid_ids);
    MAP_FOREACH_VALUE(int*, id, void*, data, freeInt, player_stats) {
        if (*id <= INVALID_ID) {
            invalid_ids[index++] = *id;
        }
    }
    for (int i = 0; i < index; ++i) {
        MapResult result = mapRemove(player_stats, invalid_ids + i);
        assert(result == MAP_SUCCESS);
    }
    free(invalid_ids);
    return player_stats;
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
    // Initialize scores.
    Map scores = mapCreate((copyMapDataElements) &copyInt, (copyMapKeyElements) &copyInt,
                           (freeMapDataElements) &freeInt,
                           (freeMapKeyElements) &freeInt, (compareMapKeyElements) &compareInt);
    if (scores == NULL) {
        return CHESS_OUT_OF_MEMORY;
    }
    MAP_FOREACH(int*, player, players_to_stats) {
        PlayerStats player_stats = mapGet(players_to_stats, player);
        int score = (player_stats->num_draws) * 1 + (player_stats->num_wins) * 2;
        mapPut(scores, player, &score);
        freeInt(player);
    }

    // Take the player with highest score.
    FILTER_FIRST(mapGetAux);

    assert(mapGetSize(scores) > 1);

    // Take the player with fewest losses.
    int *least_losses_temp = mapGetFirst(scores);
    ASSERT_NOT_NULL(least_losses_temp);
    int least_losses = *least_losses_temp;
    freeInt(least_losses_temp);
    MAP_FOREACH(int*, player, scores) {
        if (getNumOfLosses(players_to_stats, &least_losses) >= getNumOfLosses(players_to_stats, player)) {
            least_losses = *player;
        }
        freeInt(player);
    }
    MAP_FOREACH(int*, player, players_to_stats) {
        if (mapContains(scores, player) == false) {
            freeInt(player);
            continue;
        }
        if (getNumOfLosses(players_to_stats, player) > getNumOfLosses(players_to_stats, &least_losses)) {
            mapRemove(scores, player);
        }
        freeInt(player);
    }
    if (mapGetSize(scores) == 1) {
        int *winner = mapGetFirst(scores);
        *result = *winner;
        freeInt(winner);
        mapDestroy(scores);
        mapDestroy(players_to_stats);
        return CHESS_SUCCESS;
    }

    assert(mapGetSize(scores) > 1);
    // Take the player with most wins.
    FILTER_FIRST(getNumOfWinsAux);

    // Take the player with the smallest ID.
    int *winner = mapGetFirst(scores);
    *result = *winner;
    freeInt(winner);
    mapDestroy(scores);
    mapDestroy(players_to_stats);
    return CHESS_SUCCESS;
}

ChessResult countGamesPerPlayer(Tournament tournament, int player_id, int *result) {
    ASSERT_NOT_NULL(result);
    *result = 0;
    if (player_id == INVALID_ID) {
        return CHESS_SUCCESS;
    }
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
    if (mapContains(stats, &player_1_id) == false && player_1_id > INVALID_ID) {
        *result += 1;
    }
    if (mapContains(stats, &player_2_id) == false && player_2_id > INVALID_ID) {
        *result += 1;
    }
    mapDestroy(stats);
    return CHESS_SUCCESS;
}

ChessResult gameCreate(
        Tournament tournament, int first_player, int second_player, Winner winner, int play_time) {
    ASSERT_NOT_NULL(tournament);
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
    if ((gamesPerPlayer1 >= tournament->max_games_per_player && first_player > INVALID_ID) ||
        gamesPerPlayer2 >= tournament->max_games_per_player && second_player > INVALID_ID) {
        assert(gamesPerPlayer1 == tournament->max_games_per_player ||
               gamesPerPlayer2 == tournament->max_games_per_player || first_player <= INVALID_ID ||
               second_player <= INVALID_ID);
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

static ChessResult removePlayer(Tournament tournament, GameKey key, int player_id) {
    GameData data = mapGet(tournament->gameByBothPlayersId, key);
    ASSERT_NOT_NULL(data);
    assert(player_id != INVALID_ID);
    bool should_remove_first_player = key->player_1_id == player_id;
    bool new_winner_if_not_ended = should_remove_first_player ? SECOND_PLAYER : FIRST_PLAYER;
    Winner winner = tournamentHasEnded(tournament) ? data->winner : new_winner_if_not_ended;
    int player_1_id = should_remove_first_player ? (-1) * (key->player_1_id) : key->player_1_id;
    int player_2_id = should_remove_first_player ? key->player_2_id : (-1) * (key->player_2_id);
    int time = data->duration;
    RETURN_IF_NOT_SUCCESS(convertResults(mapRemove(tournament->gameByBothPlayersId, key)));
    RETURN_IF_NOT_SUCCESS(gameCreate(tournament, player_1_id, player_2_id, winner, time));
    return CHESS_SUCCESS;
}

static void printTournament(Tournament tournament) {
    printf("--Data for tournament...--\n");
    FOREACH_GAME {
        printf("p1: %d, p2: %d\n", gameId->player_1_id, gameId->player_2_id);
    }
    printf("==End data for tournament...==\n");
}

ChessResult tournamentRemovePlayer(Tournament tournament, int player_id) {
    ASSERT_NOT_NULL(tournament);
    assert(player_id > INVALID_ID);
    int size = mapGetSize(tournament->gameByBothPlayersId);
    GameKey *keys = malloc(size * sizeof(keys));
    if (keys == NULL) {
        return CHESS_OUT_OF_MEMORY;
    }
    int index = 0;
    FOREACH_GAME {
        bool isPlayer1 = compareInt(&(gameId->player_1_id), &player_id) == 0;
        bool isPlayer2 = compareInt(&(gameId->player_2_id), &player_id) == 0;
        if (!isPlayer1 && !isPlayer2) {
            continue;
        }
        assert(!isPlayer1 || !isPlayer2);
        assert(isPlayer1 || isPlayer2);
        keys[index++] = copy_game_key(gameId);
    }
    for (int i = 0; i < index; ++i) {
        ChessResult result = removePlayer(tournament, keys[i], player_id);
        assert(result == CHESS_SUCCESS);
    }
    for (int i = 0; i < index; ++i) {
        free_game_key(keys[i]);
    }
    free(keys);
    return index > 0 ? CHESS_SUCCESS : CHESS_PLAYER_NOT_EXIST;
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
