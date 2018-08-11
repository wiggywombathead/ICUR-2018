#include <stdlib.h>
#include <math.h>
#include "global.h"
#include "automaton.h"

struct automaton *init_automaton(int len, simulate_fn func, int d) {
    struct automaton *ca = malloc(sizeof(struct automaton));

    int total_cells = (int) pow(len, d);
    ca->cells = calloc(total_cells, sizeof(int));
    
    ca->len = len;
    ca->dimension = d;
    ca->sim = func;

    ca->rects = malloc(sizeof(SDL_Rect) * total_cells);

    ca->cell_width = WIN_WIDTH / len;
    ca->cell_height = WIN_HEIGHT / len;

    if (d == 1) {

        for (int i = 0; i < len; i++) {
            ca->rects[i].w = ca->cell_width;
            ca->rects[i].h = ca->cell_width;
            ca->rects[i].x = i * ca->cell_width;
            ca->rects[i].y = 0;
        }

    } else {

        for (int i = 0 ; i < len; i++) {
            for (int j = 0; j < len; j++) {
                ca->rects[i * len + j].w = ca->cell_width;
                ca->rects[i * len + j].h = ca->cell_height;
                ca->rects[i * len + j].x = j * ca->cell_width;
                ca->rects[i * len + j].y = i * ca->cell_height;
            }
        }

    }

    return ca;
}

/**
 * 111 110 101 100 011 010 001 000
 *  0   1   0   1   1   0   1   0
 */
void rule_90(struct automaton *ca) {

    int total_cells = (int) pow(ca->len, ca->dimension);
   
    /* create new array to compute states without changing current ones */
    int *new = malloc(sizeof(int) * total_cells);

    for (int i = 1; i < ca->len-1; i++) {
        new[i] = (ca->cells[i-1] + ca->cells[i+1]) % 2;
    }

    new[0] = ca->cells[1];
    new[ca->len-1] = ca->cells[ca->len-2];

    free(ca->cells);
    ca->cells = new;

}

/**
 * 111 110 101 100 011 010 001 000
 *  0   1   1   0   1   1   1   0
 */
void rule_110(struct automaton *ca) {

    int total_cells = (int) pow(ca->len, ca->dimension);

    /* create new array to compute states without changing current ones */
    int *new = malloc(sizeof(int) * total_cells);
    int result;

    for (int i = 1; i < ca->len-1; i++) {
        result = (
                (ca->cells[i] + ca->cells[i+1]) % 2) +
                !ca->cells[i-1] * ca->cells[i+1];
        result = (result > 0) ? 1 : 0;
        new[i] = result;
    }

    result = ((ca->cells[0] + ca->cells[1]) % 2) + ca->cells[1];
    result = (result > 0) ? 1 : 0;

    new[0] = result;
    new[ca->len-1] = ca->cells[ca->len-1];

    free(ca->cells);
    ca->cells = new;
}

void gol(struct automaton *ca) {

    int total_cells = (int) pow(ca->len, ca->dimension);

    int *new = calloc(total_cells, sizeof(int));
    memcpy(new, ca->cells, sizeof(int) * total_cells);

    for (int i = 0; i < ca->len; i++) {
        for (int  j = 0; j < ca->len; j++) {

            int curr = i * ca->len + j;
            int alive = 0;

            if (i > 0 && j > 0)
                alive += ca->cells[curr - ca->len - 1];

            if (i > 0)
                alive += ca->cells[curr - ca->len];

            if (i > 0 && j < ca->len-1)
                alive += ca->cells[curr - ca->len + 1];

            if (j > 0)
                alive += ca->cells[curr - 1];

            if (j < ca->len-1)
                alive += ca->cells[curr + 1];

            if (i < ca->len-1 && j > 0)
                alive += ca->cells[curr + ca->len - 1];

            if (i < ca->len-1)
                alive += ca->cells[curr + ca->len];

            if (i < ca->len-1 && j < ca->len-1)
                alive += ca->cells[curr + ca->len + 1];

            if (ca->cells[curr] == ALIVE) {

                /* death by isolation / overpopulation */
                if (alive < 2 || alive > 3)
                    new[curr] = DEAD;

            } else {

                /* reproduction */
                if (alive == 3)
                    new[curr] = ALIVE;

            }

        }
    }

    free(ca->cells);
    ca->cells = new;

}

/**
 * empty -> empty
 * head -> tail
 * tail -> conductor
 * conductor    -> head iff 2 neighbours are heads
 *              -> conductor otherwise
 */
void wireworld(struct automaton *ca) {

    int total_cells = (int) pow(ca->len, ca->dimension);

    int *new = calloc(total_cells, sizeof(int));
    memcpy(new, ca->cells, sizeof(int) * total_cells);

    for (int i = 0; i < ca->len; i++) {
        for (int  j = 0; j < ca->len; j++) {

            int curr = i * ca->len + j;
            int heads = 0;

            if (i > 0 && j > 0)
                heads += (ca->cells[curr - ca->len - 1] == HEAD);

            if (i > 0)
                heads += (ca->cells[curr - ca->len] == HEAD);

            if (i > 0 && j < ca->len-1)
                heads += (ca->cells[curr - ca->len + 1] == HEAD);

            if (j > 0)
                heads += (ca->cells[curr - 1] == HEAD);

            if (j < ca->len-1)
                heads += (ca->cells[curr + 1] == HEAD);

            if (i < ca->len-1 && j > 0)
                heads += (ca->cells[curr + ca->len - 1] == HEAD);

            if (i < ca->len-1)
                heads += (ca->cells[curr + ca->len] == HEAD);

            if (i < ca->len-1 && j < ca->len-1)
                heads += (ca->cells[curr + ca->len + 1] == HEAD);

            switch(ca->cells[curr]) {
            case EMPTY:
                new[curr] = EMPTY;
                break;
            case HEAD:
                new[curr] = TAIL;
                break;
            case TAIL:
                new[curr] = CONDUCTOR;
                break;
            case CONDUCTOR:
                if (heads == 1 || heads == 2)
                    new[curr] = HEAD;
                else
                    new[curr] = CONDUCTOR;
                break;
            default:
                new[curr] = EMPTY;
            }

        }
    }

    free(ca->cells);
    ca->cells = new;

}

void langton(struct automaton *ca) {

}
