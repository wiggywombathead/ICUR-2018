#ifndef _AUTOMATON_H
#define _AUTOMATON_H

#include <stdbool.h>
#include <SDL2/SDL.h>

struct automaton;
typedef void (*simulate_fn)(void *);

enum states {

    /* Game of Life */
    DEAD = 0,
    ALIVE = 1,

    /* BRIAN'S BRAIN */
    FIRING,
    DYING,
    
    /* WireWorld */
    HEAD,
    TAIL,
    CONDUCTOR,

    /* Langton's Ant */
    ANT,

    /* von Neumann's */
    NORTH       = 0x0,
    EAST        = 0x1,
    SOUTH       = 0x2,
    WEST        = 0x3,
    ORDINARY    = 0x4,
    SPECIAL     = 0x8,
    QUIESCENT   = 0x10,
    EXCITED     = 0x20,

    GROUND,
    S,
    S0,
    S00,
    S000,
    S01,
    S1,
    S10,
    S11,

    C00,
    C01,
    C10,
    C11,

};

struct ant {
    int x, y;
    int dx, dy;
};

struct automaton {
    int         *cells;
    int         len;
    int         dimension;
    simulate_fn sim;
    int         max_gens;
    SDL_Rect    *rects;
    int         cell_width;
    int         cell_height;
    struct ant *ant;
    bool        is_langton;
};

struct automaton *init_automaton(int len, simulate_fn func, int d);
struct ant *init_ant(int x, int y, int dx, int dy, struct automaton *);

/* elementary cellular automata */
void rule_30(void *);
void rule_54(void *);
void rule_90(void *);
void rule_110(void *);
void rule_150(void *);
void rule_182(void *);
void rule_232(void *);
void rule_250(void *);

void anneal(void *);
void brians_brain(void *);
void diamoeba(void *);
void game_of_life(void *);
void move(void *);
void langtons_ant(void *);
void life_wo_death(void *);
void replicator(void *);
void two_by_two(void *);
void wireworld(void *);

#endif
