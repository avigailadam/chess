#define CHECK_NULL(args) if ((args) == NULL) return NULL
#define NULL_ASSERT(args) assert(args != NULL)


int *copyInt(int *id) {
    CHECK_NULL(id);
    int *new_id = malloc(sizeof(*new_id));
    return new_id;
}

void freeInt(int *id) {
    NULL_ASSERT(id);
    free(id);
}

int compareInt(int *id1, int *id2) {
    NULL_ASSERT(id1);
    NULL_ASSERT(id2);
    return *id1 - *id2;
}

#ifndef CHESS_UTIL_H
#define CHESS_UTIL_H

#endif //CHESS_UTIL_H
