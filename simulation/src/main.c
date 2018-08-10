#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

#include <SDL2/SDL.h>

#include "global.h"
#include "automaton.h"

pthread_t threads[2];
pthread_mutex_t mutex;

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Event e;

extern void rule_90(struct automaton *);
extern void rule_110(struct automaton *);
extern void gol(struct automaton *);
extern void wireworld(struct automaton *);

void render(struct automaton *);

int main() {

    /* SDL initialisation */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "Error initialising SDL2: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

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

    /* program setup */
    int running = true;

    srand(time(NULL));

    int config[512];
    for (int i = 0; i < 512; i++) {
        config[i] = 0;
    }
    config[511] = 1;

    struct automaton *rule90 = init_automaton(
            sizeof(config) / sizeof(int),
            &rule_90,
            1
    );
    rule90->cells = config;

    struct automaton *rule110 = init_automaton(
            sizeof(config) / sizeof(int),
            &rule_110,
            1
    );
    rule110->cells = config;

    int *gol_conf = calloc(256 * 256, sizeof(int));
    gol_conf[32768] = gol_conf[32771] = gol_conf[33028] = gol_conf[33280] =
        gol_conf[33284] = gol_conf[33537] = gol_conf[33538] = gol_conf[33539] =
        gol_conf[33540] = ALIVE;

    // int *ww_conf = calloc(256 * 256, sizeof(int));
    // for (int i = 0; i < 256*256; i++)
    //     ww_conf[i] = EMPTY;

    int ww_conf[] = {
        0,0,5,5,5,4,3,5,5,5,0,0,    0,0,0,0,0,0,0,0,0,0,0,0,
        0,5,0,0,0,0,0,0,0,0,5,4,    3,5,5,5,0,0,0,0,0,0,0,0,
        0,0,5,5,5,5,5,5,3,4,0,0,    0,0,0,0,5,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,    0,0,0,5,5,5,5,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,    0,0,0,3,0,0,3,5,5,5,5,5,
        0,0,0,0,0,0,0,0,0,0,0,0,    0,0,0,4,4,4,3,0,0,0,0,0,
        0,0,5,5,5,4,3,5,5,5,0,0,    0,0,0,0,5,0,0,0,0,0,0,0,
        0,5,0,0,0,0,0,0,0,0,5,5,    5,5,5,5,0,0,0,0,0,0,0,0,
        0,0,3,4,5,5,5,5,5,5,0,0,    0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,    0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,    0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,    0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,    0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,    0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,    0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,    0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,    0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,    0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,    0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,    0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,    0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,    0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,    0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,    0,0,0,0,0,0,0,0,0,0,0,0,
    };

    struct automaton *wires = init_automaton(
            24,
            &wireworld,
            2
    );
    wires->cells = ww_conf;

    /*
    int gol_conf[] = {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,
        1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,
        0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,1,0,0,0,0,1,0,1,0,0,0,0,1,0,0,
        0,0,1,0,0,0,0,1,0,1,0,0,0,0,1,0,0,
        0,0,1,0,0,0,0,1,0,1,0,0,0,0,1,0,0,
        0,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0,
        0,0,1,0,0,0,0,1,0,1,0,0,0,0,1,0,0,
        0,0,1,0,0,0,0,1,0,1,0,0,0,0,1,0,0,
        0,0,1,0,0,0,0,1,0,1,0,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    };
    */

    struct automaton *conway = init_automaton(
            256,
            &gol,
            2
    );
    conway->cells = gol_conf;

    /* keep track of active automaton to simulate */
    struct automaton *active = conway;

    while (running) {

        /* poll events */
        if (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT:
                running = 0;
                break;
            case SDL_KEYDOWN:
                switch (e.key.keysym.sym) {
                case SDLK_q:
                    running = 0;
                    break;
                case SDLK_p:
                    printf("Paused\n");
                    break;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                printf("Mouse pressed!");
                break;
            }
        }

        /* render and update automaton */
        render(active);

        SDL_Delay(20);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}

void render(struct automaton *ca) {

    if (ca->dimension == 1) {

        for (int i = 0; i < ca->len; i++) {
            if (ca->cells[i] == ALIVE) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
            } else {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
            }

            SDL_RenderFillRect(renderer, &ca->rects[i]);
            ca->rects[i].y += ca->rects[i].h;
        }

    } else {

        for (int i = 0; i < ca->len; i++) {
            for (int j= 0; j < ca->len; j++) {

                int offset = i * ca->len + j;

                switch (ca->cells[offset]) {
                case DEAD:
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
                    break;
                case ALIVE:
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
                    break;
                case EMPTY:
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
                    break;
                case HEAD:
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
                    break;
                case TAIL:
                    SDL_SetRenderDrawColor(renderer, 255, 100, 0, SDL_ALPHA_OPAQUE);
                    break;
                case CONDUCTOR:
                    SDL_SetRenderDrawColor(renderer, 255, 255, 0, SDL_ALPHA_OPAQUE);
                    break;
                }

                SDL_RenderFillRect(renderer, &ca->rects[offset]);
            }
        }

    }

    SDL_RenderPresent(renderer);
    ca->sim(ca);
}
