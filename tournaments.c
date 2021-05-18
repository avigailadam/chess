#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "map.h"
#include "tournaments.h"

typedef struct tournament_t {
    Map gameByBothPlayersId;
} *Tournament;