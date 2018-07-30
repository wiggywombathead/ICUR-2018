#include <stdlib.h>
#include "global.h"
#include "automaton.h"

struct automaton *init_automaton(int len, simulate_fn func, int d) {
    struct automaton *ca = malloc(sizeof(struct automaton));
    ca->cells = calloc(len, sizeof(int));
    
    ca->len = len;
    ca->dimension = d;
    ca->sim = func;

    ca->rects = malloc(sizeof(SDL_Rect) * len);

    for (int i = 0; i < len; i++) {
        ca->rects[i].w = WIN_WIDTH / len;
        ca->rects[i].h = ca->rects[i].w;
        ca->rects[i].x = i * ca->rects[i].w;
        ca->rects[i].y = 0;
    }

    return ca;
}
