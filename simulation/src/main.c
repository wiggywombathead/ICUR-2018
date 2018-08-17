#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#include <SDL2/SDL.h>

#include "global.h"
#include "automaton.h"

#define max(a,b) ((a) >= (b) ? a : b)

/* TODO
 *  for langton's ant, pass ant coords in void * param (along with ca)
 *  render active->cells[ant] as an unused color e.g. red
 *  add ant coords to struct automaton ?
 */

float lerp(float start, float end, float t) {
    return start + t * (end - start);
}

int diag(int x1, int y1, int x2, int y2) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    return max(abs(dx), abs(dy));
}

/* milliseconds for each frame to take */
const int FRAME_INTERVAL = 1 * 1000 / FRAME_RATE;

SDL_Window *window;
SDL_Renderer *renderer;

int running = true;
int paused = false;
int mode = SIMULATE;    /* enable/disable drawing */
int paintbrush;         /* the state with which to overwrite cell */
struct automaton *active;

extern void rule_90(void *);
extern void rule_110(void *);
extern void gol(void *);
extern void wireworld(void *);
extern void langton(void *);

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

    srand(time(NULL));

    int *config = calloc(512, sizeof(int));
    for (int i = 0; i < 512; i++) {
        config[i] = rand() % 2;
    }

    struct automaton *rule90 = init_automaton(
            512,
            &rule_90,
            1,
            1000
    );
    rule90->cells = config;

    struct automaton *rule110 = init_automaton(
            256,
            &rule_110,
            1,
            1000
    );
    rule110->cells = config;

    int *ww_conf = calloc(64 * 64, sizeof(int));
    for (int i = 0; i < 64*64; i++)
        ww_conf[i] = EMPTY;

    struct automaton *wires = init_automaton(
            64,
            &wireworld,
            2,
            1000
    );
    wires->cells = ww_conf;

    int *gol_conf = calloc(128 * 128, sizeof(int));
    struct automaton *conway = init_automaton(
            128,
            &gol,
            2,
            1000
    );
    conway->cells = gol_conf;

    int *lang = calloc(128*128, sizeof(int));
    struct automaton *ant = init_automaton(
            128,
            &langton,
            2,
            1000
    );
    ant->cells = lang;

    active = wires;

    unsigned int frame_start;
    unsigned int frame_curr;
    unsigned int wait_time;

    frame_start = SDL_GetTicks();

    while (running) {

        handle_input();

        /* render automaton */
        render(active);

        if (!paused) {
            /* update automaton */
            active->sim(active);
        }

        frame_curr = SDL_GetTicks();

        if (frame_curr < frame_start + FRAME_INTERVAL) {
            wait_time = frame_start + FRAME_INTERVAL - frame_curr;
            SDL_Delay(wait_time);
        }

        frame_start = frame_curr;

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
                case ANT:
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
                    break;
                }

                SDL_RenderFillRect(renderer, &ca->rects[offset]);
            }
        }

    }

    SDL_RenderPresent(renderer);
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

    /* for mouse coords */
    static int x1, y1, x2, y2;

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
            case SDLK_LSHIFT:
                SDL_GetMouseState(&x1, &y1);
                break;
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
                paintbrush = HEAD;
                break;
            case SDLK_3:
                paintbrush = CONDUCTOR;
                break;
            case SDLK_4:
                break;
            case SDLK_5:
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
                paintbrush = ALIVE;
            }
            break;
        case SDL_KEYUP:
            switch (e.key.keysym.sym) {
            case SDLK_LSHIFT:
                SDL_GetMouseState(&x2, &y2);

                int ai, aj, bi, bj;
                ai = x1 / active->cell_width;
                aj = y1 / active->cell_height;
                bi = x2 / active->cell_width;
                bj = y2 / active->cell_height;

                float N = (float) diag(ai, aj, bi, bj);

                for (int i = 0; i <= N; i++) {
                    float t = (N == 0) ? 0.f : i / N;
                    n_i = round(lerp(ai, bi, t));
                    n_j = round(lerp(aj, bj, t));
                    active->cells[n_j*active->len + n_i] = paintbrush;
                }

                break;
            }

        }
    }

    Uint8* keystate = SDL_GetKeyboardState(NULL);

    /* continuous response keys */
    if (keystate[SDL_SCANCODE_LSHIFT]) {
    }

    if(keystate[SDLK_RIGHT]) {
    }

    if(keystate[SDLK_UP]) {
    }

    if(keystate[SDLK_DOWN]) {
    }

}
