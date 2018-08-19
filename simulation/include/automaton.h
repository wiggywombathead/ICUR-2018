#ifndef _AUTOMATON_H
#define _AUTOMATON_H

#include <stdbool.h>
#include <SDL2/SDL.h>

struct automaton;
typedef void (*simulate_fn)(void *);

enum direction {
    UP, RIGHT, DOWN, LEFT
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

void rule_30(void *);
void rule_90(void *);
void rule_110(void *);
void brians_brain(void *);
void game_of_life(void *);
void wireworld(void *);
void langtons_ant(void *);

#endif
