#ifndef _AUTOMATON_H
#define _AUTOMATON_H

#include <SDL2/SDL.h>

struct automaton;
typedef void (*simulate_fn)(struct automaton *);

enum direction {
    UP, RIGHT, DOWN, LEFT
};

struct automaton {
    int *cells;
    int len;
    int dimension;
    SDL_Rect *rects;
    int cell_width;
    int cell_height;
    simulate_fn sim;
};

struct ant {
    int pos;
    int direction;
};

struct automaton *init_automaton(int len, simulate_fn func, int d);

#endif
