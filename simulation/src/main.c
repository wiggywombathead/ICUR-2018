#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#include <SDL2/SDL.h>

#include "global.h"
#include "automaton.h"

int max_gens = 1024;

extern void rule_90(struct automaton *);
extern void gol(struct automaton *);

void render(SDL_Renderer *r, struct automaton *);

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
    int is_running = true;
    int current_gen = 0;

    srand(time(NULL));

    int config[1024];
    for (int i = 0; i < 1024; i++) {
        config[i] = rand() % 2;
    }

    struct automaton *rule1 = init_automaton(
            sizeof(config) / sizeof(int),
            &rule_90,
            1
    );
    rule1->cells = config;

    int gol_conf[] = {
        0,0,0,0,
        0,1,0,0,
        0,0,0,0,
        0,0,0,0
    };
    struct automaton *conway = init_automaton(
            sqrt(sizeof(gol_conf) / sizeof(int)),
            &gol,
            2
    );
    conway->cells = gol_conf;

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

        /* render and update automaton */
        render(renderer, conway);

        SDL_Delay(100);
        current_gen++;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}

void render(SDL_Renderer *r, struct automaton *ca) {

    if (ca->dimension == 1) {

        for (int i = 0; i < ca->len; i++) {
            if (ca->cells[i] == ALIVE) {
                SDL_SetRenderDrawColor(r, 255, 255, 255, SDL_ALPHA_OPAQUE);
            } else {
                SDL_SetRenderDrawColor(r, 0, 0, 0, SDL_ALPHA_OPAQUE);
            }

            SDL_RenderFillRect(r, &ca->rects[i]);
            ca->rects[i].y += ca->rects[i].h;
        }

    } else {

        SDL_SetRenderDrawColor(r, 0, 0, 0, SDL_ALPHA_OPAQUE);

        for (int i = 0; i < ca->len; i++) {
            for (int j= 0; j < ca->len; j++) {

                int offset = i * ca->len + j;

                if (ca->cells[offset] == ALIVE) {
                    SDL_SetRenderDrawColor(r, 255, 255, 255, SDL_ALPHA_OPAQUE);
                } else {
                    SDL_SetRenderDrawColor(r, 0, 0, 0, SDL_ALPHA_OPAQUE);
                }

                SDL_RenderFillRect(r, &ca->rects[offset]);
            }
        }

    }

    SDL_RenderPresent(r);
    ca->sim(ca);
}
