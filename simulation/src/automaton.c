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

    int c_width = WIN_WIDTH / len;

    if (d == 1) {

        for (int i = 0; i < len; i++) {
            ca->rects[i].w = c_width;
            ca->rects[i].h = c_width;
            ca->rects[i].x = i * c_width;
            ca->rects[i].y = 0;
        }

    } else {

        int c_height = WIN_HEIGHT / len;

        for (int i = 0 ; i < len; i++) {
            for (int j = 0; j < len; j++) {
                ca->rects[i * len + j].w = c_width;
                ca->rects[i * len + j].h = c_height;
                ca->rects[i * len + j].x = j * c_width;
                ca->rects[i * len + j].y = i * c_height;
            }
        }

    }

    return ca;
}

void rule_90(struct automaton *ca) {

    int total_cells = (int) pow(ca->len, ca->dimension);
   
    /* create new array to compute states without changing current ones */
    int *new = malloc(sizeof(int) * total_cells);

    for (int i = 1; i < ca->len-1; i++) {
        new[i] = (ca->cells[i-1] + ca->cells[i+1]) % 2;
    }

    new[0] = ca->cells[1];
    new[ca->len-1] = ca->cells[ca->len-2];

    ca->cells = new;

}

void gol(struct automaton *ca) {

    int total_cells = (int) pow(ca->len, ca->dimension);

    int *new = calloc(total_cells, sizeof(int));
    memcpy(new, ca->cells, sizeof(int) * total_cells);

    printf("Current:\n");
    for (int i = 0; i < ca->len; i++) {
        for (int  j = 0; j < ca->len; j++) {
            printf("%d ", ca->cells[i*ca->len+j]);
        }
        printf("\n");
    }
    printf("----------\nAlive:\n");

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

            if (i < ca->len-1 && j < 0)
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
            printf("%d ", alive);

        }
        printf("\n");
    }

    ca->cells = new;

}
