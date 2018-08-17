#ifndef _AUTOMATON_H
#define _AUTOMATON_H

#include <SDL2/SDL.h>

struct automaton;
typedef void (*simulate_fn)(void *);

enum direction {
    UP, RIGHT, DOWN, LEFT
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
};

struct ant {
    int x, y;
    int dx, dy;
};

struct langton_config {
    struct ant *ant;
    struct automaton *automaton;
};

struct automaton *init_automaton(int len, simulate_fn func, int d, int g);

void rule_90(void *);
void rule_110(void *);
void gol(void *);
void wireworld(void *);
void langton(void *);

#endif
