#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <time.h>

#include "global.h"
#include "automaton.h"

int max_gens = 1024;

void rule_01(struct automaton *ca) {
   
    /* create new array to compute states without changing current ones */
    int *new = malloc(sizeof(int) * ca->len);

    for (int i = 1; i < ca->len-1; i++) {
        new[i] = (ca->cells[i-1] + ca->cells[i+1]) % 2;
    }

    new[0] = ca->cells[1];
    new[ca->len-1] = ca->cells[ca->len-2];

    ca->cells = new;

}
/*
void gol(struct automaton *ca) {

    int *new = malloc(sizeof(int) * ca->len);
    for (int i = 0; i < ca->len; i++) {
        for (int j = 0; j < ca->len; j++) {

            int alive = 0;

            alive += ca->cells[i-1][j-1];
            alive += ca->cells[i-1][j];
            alive += ca->cells[i-1][j+1];
            alive += ca->cells[i][j-1];
            alive += ca->cells[i][j+1];
            alive += ca->cells[i+1][j-1];
            alive += ca->cells[i+1][j];
            alive += ca->cells[i+1][j+1];

        }
    }

}
*/

void render(SDL_Renderer *r, struct automaton *ca) {

    /* draw to screen */
    for (int i = 0; i < ca->len; i++) {
        if (ca->cells[i] == ALIVE) {
            SDL_SetRenderDrawColor(r, 255, 255, 255, SDL_ALPHA_OPAQUE);
        } else {
            SDL_SetRenderDrawColor(r, 0, 0, 0, SDL_ALPHA_OPAQUE);
        }

        SDL_RenderFillRect(r, &ca->rects[i]);
        ca->rects[i].y += ca->rects[i].h;
    }
}

int main() {

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "Error initialising SDL2: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Window *window;
    SDL_Renderer *renderer;
    
    window = SDL_CreateWindow(
            "Cellular Automata",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            WIN_WIDTH,
            WIN_HEIGHT,
            SDL_WINDOW_SHOWN
    );
    int win_width, win_height;
    SDL_GetWindowSize(window, &win_width, &win_height);

    if (window == NULL) {
        fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    renderer = SDL_CreateRenderer(
            window,
            -1,
            SDL_RENDERER_ACCELERATED
    );

    if (renderer == NULL) {
        fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Event e;
    int is_running = 1;
    int current_gen = 0;

    srand(time(NULL));
    int config[1024];
    for (int i = 0; i < 1024; i++) {
        // config[i] = rand() % 2;
        if (rand() % 10 == 0)
            config[i] = ALIVE;
    }

    printf("%ld\n", sizeof(config)/sizeof(config[0]));
    struct automaton *rule1 = init_automaton(
            sizeof(config) / sizeof(int),
            &rule_01,
            1
    );
    rule1->cells = config;

    while (is_running) {

        /* poll events */
        SDL_PollEvent(&e);
        switch (e.type) {
        case SDL_QUIT:
            is_running = 0;
            break;
        case SDL_KEYDOWN:
            switch (e.key.keysym.sym) {
            case SDLK_q:
                is_running = 0;
                break;
            case SDLK_p:
                printf("Paused\n");
                break;
            }
            break;
        }

        if (rule1->rects[0].y + rule1->rects[0].h > win_height)
            continue;

        /* render */
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);

        render(renderer, rule1);

        SDL_RenderPresent(renderer);
        /* update objects */
        rule1->sim(rule1);
        current_gen++;
        
        SDL_Delay(6);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
