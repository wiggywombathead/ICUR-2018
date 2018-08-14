#include <stdlib.h>
#include <math.h>
#include "global.h"
#include "automaton.h"

struct automaton *init_automaton(int len, simulate_fn func, int d, int g) {
    struct automaton *ca = malloc(sizeof(struct automaton));

    int total_cells = (int) pow(len, d);
    ca->cells = calloc(total_cells, sizeof(int));
    
    ca->len = len;
    ca->sim = func;
    ca->dimension = d;
    ca->max_gens = g;

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
void rule_90(void *data) {

    struct automaton *ca = (struct automaton *) data;
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
void rule_110(void *data) {

    struct automaton *ca = (struct automaton *) data;
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

void gol(void *data) {

    struct automaton *ca = (struct automaton *) data;
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
void wireworld(void *data) {

    struct automaton *ca = (struct automaton *) data;
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

/**
 * black -> white, turn left
 * white -> black, turn right
 * move ant forward
 */
void langton(void *data) {

    struct langton_config *lc = (struct langton_config *) data;

    struct ant *ant = lc->ant;
    struct automaton *ca = lc->automaton;

    int x = ant->x;
    int y = ant->y;
    int dx = ant->dx;
    int dy = ant->dy;

    int offset = y * ca->len + x;

    // int total_cells = (int) pow(ca->len, ca->dimension);

    // int *new = calloc(total_cells, sizeof(int));
    // memcpy(new, ca->cells, sizeof(int) * total_cells);

    /*
     *  a.b = |a||b| cos theta
     *  a.b = 0 (orthogonal)
     *
     *  we have a, want b
     *  ax.bx + ay.by = 0
     *  bx = -(ay.by)/ax
     *
     *  |b| = 1 = sqrt(b.b) = pow(bx,2)+pow(by,2)
     *  bx = sqrt(1 - pow(by,2))
     *
     *  [ALGEBRA]
     *
     *  by = +/- sqrt(1/(1+pow(ay,2)/pow(ax,2)))
     */

    int k;

    switch (offset) {
    case DEAD:
        ca->cells[offset] = ALIVE;
        k = dx;
        dx = -dy;
        dy = k;
        break;
    case ALIVE:
        ca->cells[offset] = DEAD;
        k = dy;
        dy = -dx;
        dx = k;
        break;
    }

    ant += (dy*ca->len + dx);

}
