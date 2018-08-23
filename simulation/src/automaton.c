#include <stdlib.h>
#include <math.h>
#include "global.h"
#include "automaton.h"

struct automaton *init_automaton(int len, simulate_fn func, int d) {
    struct automaton *ca = malloc(sizeof(struct automaton));

    int total_cells = (int) pow(len, d);
    ca->cells = calloc(total_cells, sizeof(int));
    
    ca->len = len;
    ca->sim = func;
    ca->dimension = d;

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

    ca->is_langton = false;

    return ca;
}

struct ant *init_ant(int x, int y, int dx, int dy, struct automaton *ca) {
    struct ant *ant = malloc(sizeof(struct ant));
    ant->x = x;
    ant->y = y;
    ant->dx = dx;
    ant->dy = dy;

    ca->ant = ant;
    ca->is_langton = true;

    return ant;
}

/**
 * 111 110 101 100 011 010 001 000
 *  0   0   0   1   1   1   1   0
 */
void rule_30(void *data) {

    struct automaton *ca = (struct automaton *) data;
    int total_cells = (int) pow(ca->len, ca->dimension);
   
    /* create new array to compute states without changing current ones */
    int *new = malloc(sizeof(int) * total_cells);

    for (int i = 1; i < ca->len-1; i++) {
        int a, b, c;
        a = ca->cells[i-1];
        b = ca->cells[i];
        c = ca->cells[i+1];
        
        new[i] = (~a & (b | c)) | (a & ~b & ~c);
    }

    new[0] = ca->cells[0] | ca->cells[1];
    new[ca->len-1] = ca->cells[ca->len-2] ^ ca->cells[ca->len-1];

    for (int i = 0; i < ca->len; i++) {
        ca->rects[i].y += ca->rects[i].h;
    }

    ca->cells = new;
}

/**
 * 111 110 101 100 011 010 001 000
 *  0   0   1   1   0   1   1   0
 */
void rule_54(void *data) {

    struct automaton *ca = (struct automaton *) data;
    int total_cells = (int) pow(ca->len, ca->dimension);
   
    /* create new array to compute states without changing current ones */
    int *new = malloc(sizeof(int) * total_cells);

    for (int i = 1; i < ca->len-1; i++) {
        int a, b, c;
        a = ca->cells[i-1];
        b = ca->cells[i];
        c = ca->cells[i+1];
        
        new[i] = (~a & b & ~c) | (~b & (a | c));
    }

    new[0] = ca->cells[0] ^ ca->cells[1];
    new[ca->len-1] = ca->cells[ca->len-2] ^ ca->cells[ca->len-1];

    for (int i = 0; i < ca->len; i++) {
        ca->rects[i].y += ca->rects[i].h;
    }

    ca->cells = new;
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
        new[i] = ca->cells[i-1] ^ ca->cells[i+1];
    }

    new[0] = ca->cells[1];
    new[ca->len-1] = ca->cells[ca->len-2];

    for (int i = 0; i < ca->len; i++) {
        ca->rects[i].y += ca->rects[i].h;
    }

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

    for (int i = 1; i < ca->len-1; i++) {
        int a, b, c;
        a = ca->cells[i-1];
        b = ca->cells[i];
        c = ca->cells[i+1];

        new[i] = (~a & c) | (b ^ c);
    }

    new[0] = ca->cells[1] | (ca->cells[0] ^ ca->cells[1]);
    new[ca->len-1] = ca->cells[ca->len-1];

    for (int i = 0; i < ca->len; i++) {
        ca->rects[i].y += ca->rects[i].h;
    }

    ca->cells = new;
}

void rule_150(void *data) {

    struct automaton *ca = (struct automaton *) data;
    int total_cells = (int) pow(ca->len, ca->dimension);
   
    /* create new array to compute states without changing current ones */
    int *new = malloc(sizeof(int) * total_cells);

    for (int i = 1; i < ca->len-1; i++) {
        int a, b, c;
        a = ca->cells[i-1];
        b = ca->cells[i];
        c = ca->cells[i+1];
        
        new[i] = (c & ((a & b) | ~(a | b))) | (~c & (a ^ b));
    }

    new[0] = ca->cells[0] ^ ca->cells[1];
    new[ca->len-1] = ca->cells[ca->len-2] ^ ca->cells[ca->len-1];

    for (int i = 0; i < ca->len; i++) {
        ca->rects[i].y += ca->rects[i].h;
    }

    ca->cells = new;
}

void rule_182(void *data) {

    struct automaton *ca = (struct automaton *) data;
    int total_cells = (int) pow(ca->len, ca->dimension);
   
    /* create new array to compute states without changing current ones */
    int *new = malloc(sizeof(int) * total_cells);

    for (int i = 1; i < ca->len-1; i++) {
        int a, b, c;
        a = ca->cells[i-1];
        b = ca->cells[i];
        c = ca->cells[i+1];
        
        new[i] = (~a & b & ~c) | (a & (~b | c)) | (~b & c);
    }

    new[0] = ca->cells[0] ^ ca->cells[1];
    new[ca->len-1] = ca->cells[ca->len-2] ^ ca->cells[ca->len-1];

    for (int i = 0; i < ca->len; i++) {
        ca->rects[i].y += ca->rects[i].h;
    }

    ca->cells = new;
}

void rule_232(void *data) {

    struct automaton *ca = (struct automaton *) data;
    int total_cells = (int) pow(ca->len, ca->dimension);
   
    /* create new array to compute states without changing current ones */
    int *new = malloc(sizeof(int) * total_cells);

    for (int i = 1; i < ca->len-1; i++) {
        int a, b, c;
        a = ca->cells[i-1];
        b = ca->cells[i];
        c = ca->cells[i+1];

        new[i] = (a & b) | (b & c) | (a & c);
    }

    new[0] = ca->cells[0] & ca->cells[1];
    new[ca->len-1] = ca->cells[ca->len-2] & ca->cells[ca->len-1];

    for (int i = 0; i < ca->len; i++) {
        ca->rects[i].y += ca->rects[i].h;
    }

    ca->cells = new;
}

void rule_250(void *data) {

    struct automaton *ca = (struct automaton *) data;
    int total_cells = (int) pow(ca->len, ca->dimension);
   
    /* create new array to compute states without changing current ones */
    int *new = malloc(sizeof(int) * total_cells);

    for (int i = 1; i < ca->len-1; i++) {
        new[i] = ca->cells[i-1] | ca->cells[i+1];
    }

    new[0] = ca->cells[1];
    new[ca->len-1] = ca->cells[ca->len-2];

    for (int i = 0; i < ca->len; i++) {
        ca->rects[i].y += ca->rects[i].h;
    }

    ca->cells = new;
}

void brians_brain(void *data) {

    struct automaton *ca = (struct automaton *) data;
    int total_cells = (int) pow(ca->len, ca->dimension);

    int *new = calloc(total_cells, sizeof(int));
    memcpy(new, ca->cells, sizeof(int) * total_cells);

    for (int i = 0; i < ca->len; i++) {
        for (int  j = 0; j < ca->len; j++) {

            int curr = i * ca->len + j;
            int firing = 0;

            if (ca->cells[curr] == FIRING)
                new[curr] = DYING;

            if (ca->cells[curr] == DYING)
                new[curr] = DEAD;

            if (ca->cells[curr] == DEAD) {
                if (i > 0 && j > 0)
                    firing += (ca->cells[curr - ca->len - 1] == FIRING);

                if (i > 0)
                    firing += (ca->cells[curr - ca->len] == FIRING);

                if (i > 0 && j < ca->len-1)
                    firing += (ca->cells[curr - ca->len + 1] == FIRING);

                if (j > 0)
                    firing += (ca->cells[curr - 1] == FIRING);

                if (j < ca->len-1)
                    firing += (ca->cells[curr + 1] == FIRING);

                if (i < ca->len-1 && j > 0)
                    firing += (ca->cells[curr + ca->len - 1] == FIRING);

                if (i < ca->len-1)
                    firing += (ca->cells[curr + ca->len] == FIRING);

                if (i < ca->len-1 && j < ca->len-1)
                    firing += (ca->cells[curr + ca->len + 1] == FIRING);

                if (firing == 2)
                    new[curr] = FIRING;

            }

        }
    }

    free(ca->cells);
    ca->cells = new;
}

void game_of_life(void *data) {

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
            case DEAD:
                new[curr] = DEAD;
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
                new[curr] = DEAD;
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
void langtons_ant(void *data) {

    struct automaton *ca = (struct automaton *) data;

    int x = ca->ant->x;
    int y = ca->ant->y;
    int dx = ca->ant->dx;
    int dy = ca->ant->dy;

    int offset = y * ca->len + x;

    switch (ca->cells[offset]) {
    case DEAD:
        ca->cells[offset] = ALIVE;
        ca->ant->dx = dy;
        ca->ant->dy = -dx;
        break;
    case ALIVE:
        ca->cells[offset] = DEAD;
        ca->ant->dx = -dy;
        ca->ant->dy = dx;
        break;
    }

    ca->ant->x += ca->ant->dx;
    ca->ant->y += ca->ant->dy;
}

/**
 * B1357/S1357
 */
void replicator(void *data) {

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

                /* survival / death */
                switch (alive) {
                case 1:
                case 3:
                case 5:
                case 7:
                    new[curr] = ALIVE;
                    break;
                default:
                    new[curr] = DEAD;
                }

            } else {

                /* reproduction */
                switch (alive) {
                case 1:
                case 3:
                case 5:
                case 7:
                    new[curr] = ALIVE;
                    break;
                default:
                    new[curr] = DEAD;
                }

            }

        }
    }

    free(ca->cells);
    ca->cells = new;
}

/**
 * B35678/S5678
 */
void diamoeba(void *data) {

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

                /* survival / death */
                switch (alive) {
                case 5:
                case 6:
                case 7:
                case 8:
                    new[curr] = ALIVE;
                    break;
                default:
                    new[curr] = DEAD;
                }

            } else {

                /* reproduction */
                switch (alive) {
                case 3:
                case 5:
                case 6:
                case 7:
                case 8:
                    new[curr] = ALIVE;
                    break;
                default:
                    new[curr] = DEAD;
                }

            }

        }
    }

    free(ca->cells);
    ca->cells = new;
}

/**
 * John von Neumann's CA
 *
 * States:
 *  Ground state:
 *      U
 *
 *  Sensitised states:
 *      S
 *      S0
 *      S00
 *      S000
 *      S01
 *      S1
 *      S10
 *      S11
 *
 *  Confluent states:
 *      C00
 *      C01
 *      C10
 *      C11
 *
 *  Ordinary transmission states:
 *      Noe
 *      Noq
 *      Eoe
 *      Eoq
 *      Soe
 *      Soq
 *      Woe
 *      Woq
 *
 *  Special transmission states:
 *      Nse
 *      Nsq
 *      Ese
 *      Esq
 *      Sse
 *      Ssq
 *      Wse
 *      Wsq
 *
 * Construction Rules
 *  U => S
 *  S -> S0
 *      S0 -> S00
 *          S00 -> S000
 *              S000 -> Eoe
 *              S000 => Noe
 *          S00 => Woe
 *      S0 => S01
 *          S01 -> Soe
 *          S01 => Ese
 *  S => S1
 *      S1 -> S10
 *          S10 -> Nse
 *          S10 => Wse
 *      S1 => S11
 *          S11 -> Sse
 *          S11 => C00
 *
 * Destruction Rules
 *  special => confluent: confluent ~> U
 *  special => ordinary: ordinary ~> U
 *  ordinary => special: special ~> U
 */

void von_neumanns(void *data) {

    struct automaton *ca = (struct automaton *) data;
    int total_cells = (int) pow(ca->len, ca->dimension);

    int *new = calloc(total_cells, sizeof(int));
    memcpy(new, ca->cells, sizeof(int) * total_cells);

    for (int i = 0; i < ca->len; i++) {
        for (int  j = 0; j < ca->len; j++) {

            int curr = i * ca->len + j;

            int neighbourhood[4];
            int required_dir[] = {SOUTH, WEST, NORTH, EAST};

            int dir;

            if (i > 0) {
                neighbourhood[0] = ca->cells[curr - ca->len];
            } else {
                neighbourhood[0] = -1;
            }

            if (j > 0) {
                neighbourhood[1] = ca->cells[curr - 1];
            } else {
                neighbourhood[1] = -1;
            }

            if (j < ca->len-1) {
                neighbourhood[2] = ca->cells[curr + 1];
            } else {
                neighbourhood[2] = -1;
            }

            if (i < ca->len-1) {
                neighbourhood[3] = ca->cells[curr + ca->len];
            } else {
                neighbourhood[3] = -1;
            }

            switch(ca->cells[curr]) {
            case NORTH | ORDINARY | QUIESCENT:
            case NORTH | ORDINARY | EXCITED:
            case NORTH | SPECIAL | QUIESCENT:
            case NORTH | SPECIAL | EXCITED:

            case EAST | ORDINARY | QUIESCENT:
            case EAST | ORDINARY | EXCITED:
            case EAST | SPECIAL | QUIESCENT:
            case EAST | SPECIAL | EXCITED:

            case SOUTH | ORDINARY | QUIESCENT:
            case SOUTH | ORDINARY | EXCITED:
            case SOUTH | SPECIAL | QUIESCENT:
            case SOUTH | SPECIAL | EXCITED:

            case WEST | ORDINARY | QUIESCENT:
            case WEST | ORDINARY | EXCITED:
            case WEST | SPECIAL | QUIESCENT:
            case WEST | SPECIAL | EXCITED:
                dir = ca->cells[curr] & 0x3;
                for (int i = 0; i < 4; i++) {
                    int input_dir = neighbourhood[i] & 0x3;
                    int is_excited = neighbourhood[i] & EXCITED;

                    if (input_dir == required_dir[i] && is_excited) {
                        new[curr] = EXCITED;
                        break;
                    }
                }

            }

        }
    }

    free(ca->cells);
    ca->cells = new;
}
