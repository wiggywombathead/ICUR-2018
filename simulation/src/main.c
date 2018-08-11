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

int running = true;
int paused = false;
int mode = SIMULATE;    /* enable/disable drawing */
int paintbrush;         /* the state with which to overwrite cell */
struct automaton *active;

extern void rule_90(struct automaton *);
extern void rule_110(struct automaton *);
extern void gol(struct automaton *);
extern void wireworld(struct automaton *);

void render(struct automaton *);
void handle_input(void);
void print_help(void);

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

    int *ww_conf = calloc(64 * 64, sizeof(int));
    for (int i = 0; i < 64*64; i++)
        ww_conf[i] = EMPTY;

    struct automaton *wires = init_automaton(
            64,
            &wireworld,
            2
    );
    wires->cells = ww_conf;

    struct automaton *conway = init_automaton(
            256,
            &gol,
            2
    );
    conway->cells = gol_conf;

    active = wires;

    while (running) {

        handle_input();

        /* render automaton */
        render(active);

        if (!paused) {
            /* update automaton */
            active->sim(active);
            SDL_Delay(20);
        }

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
                    SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
                    break;
                case TAIL:
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
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
    // ca->sim(ca);
}

void print_help(void) {
    char help[] =
        "States:\n\t"
        "0 - DEAD\n\t"
        "1 - ALIVE\n\t"
        "2 - EMPTY\n\t"
        "3 - HEAD\n\t"
        "4 - TAIL\n\t"
        "5 - CONDUCTOR\n\t"
        "6 - \n\t"
        "7 - \n\t"
        "8 - \n\t"
        "9 - \n";

    printf("%s", help);
}

void handle_input() {

    // Uint8 *keystate = SDL_GetKeyState(NULL);

    /*
    if(keystate[SDLK_LEFT])
    if(keystate[SDLK_RIGHT])
    if(keystate[SDLK_UP])
    if(keystate[SDLK_DOWN])
    */

    /* keep track of which cell to modify in DRAW mode */
    int n_i, n_j;

    SDL_Event e;

    /* poll events */
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_QUIT:
            running = 0;
            break;
        case SDL_MOUSEBUTTONDOWN:
            switch (e.button.button) {
            case SDL_BUTTON_LEFT:
                if (mode == DRAW) {
                    n_i = e.motion.x / active->cell_width;
                    n_j = e.motion.y / active->cell_height;

                    active->cells[n_j*active->len + n_i] = paintbrush;
                }

                break;
            case SDL_BUTTON_RIGHT:
                if (mode == DRAW) {
                    n_i = e.motion.x / active->cell_width;
                    n_j = e.motion.y / active->cell_height;

                    active->cells[n_j*active->len + n_i] = DEAD;
                }
                break;
            }
            break;
        case SDL_KEYDOWN:
            switch (e.key.keysym.sym) {
            case SDLK_h:
                print_help();
                break;
            case SDLK_q:
                running = 0;
                break;
            case SDLK_p:
                if (paused)
                    printf("Resuming...\n");
                else
                    printf("Paused\n");
                paused = !paused;
                break;
            case SDLK_m:
                mode = (mode == DRAW) ? SIMULATE : DRAW;
                printf("%s mode\n", (mode == DRAW) ? "Draw" : "Simulate");
                break;
            case SDLK_0:
                paintbrush = DEAD;
                break;
            case SDLK_1:
                paintbrush = ALIVE;
                break;
            case SDLK_2:
                paintbrush = EMPTY;
                break;
            case SDLK_3:
                paintbrush = HEAD;
                break;
            case SDLK_4:
                paintbrush = TAIL;
                break;
            case SDLK_5:
                paintbrush = CONDUCTOR;
                break;
            case SDLK_6:
                break;
            case SDLK_7:
                break;
            case SDLK_8:
                break;
            case SDLK_9:
                break;
            default:
                paintbrush = DEAD;
            }
            break;
        }
    }

}
