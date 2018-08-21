#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "global.h"
#include "automaton.h"

#define max(a,b) ((a) >= (b) ? a : b)

/* milliseconds for each frame to take */
const int FRAME_INTERVAL = 1 * 1000 / FRAME_RATE;

SDL_Window *window;
SDL_Renderer *renderer;

bool running = true;
bool paused = false;
bool step = false;
int paintbrush = ALIVE;     /* the state with which to overwrite cell */

/* simulation speed */
int speed = 0;
int gens = 0;

struct automaton *rule30;
struct automaton *rule54;
struct automaton *rule90;
struct automaton *rule110;
struct automaton *rule150;
struct automaton *rule182;
struct automaton *rule232;
struct automaton *rule250;

struct automaton *conway;
struct automaton *wires;
struct automaton *brian;
struct automaton *langton;

struct automaton *active;

void render(struct automaton *);
void handle_input(void);

void switch_automaton(struct automaton *);

float lerp(float, float, float);
int diag(int, int, int, int);

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

    if(TTF_Init() == -1) {
        fprintf(stderr, "Error on TTF_Init(): %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }

    int ticks = 0;
    char gen_str[12];
    int gen_len = 1;
    int last_gen_len = gen_len;

    /* text setup */
    TTF_Font *font = TTF_OpenFont("consolas.ttf", 144);
    if(!font) {
        fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
    }

    srand(time(NULL));

    /* automata initialisation */
    int *config = calloc(400*400, sizeof(int));

    rule30 = init_automaton(400, &rule_30, 1);
    rule30->cells = config;

    rule54 = init_automaton(400, &rule_54, 1);
    rule54->cells = config;

    rule90 = init_automaton(400, &rule_90, 1);
    rule90->cells = config;

    rule110 = init_automaton(400, &rule_110, 1);
    rule110->cells = config;
    
    rule150 = init_automaton(400, &rule_150, 1);
    rule150->cells = config;

    rule182 = init_automaton(400, &rule_182, 1);
    rule182->cells = config;

    rule232 = init_automaton(400, &rule_232, 1);
    rule232->cells = config;

    rule250 = init_automaton(400, &rule_250, 1);
    rule250->cells = config;

    wires = init_automaton(200, &wireworld, 2);
    wires->cells = config;

    conway = init_automaton(200, &game_of_life, 2);
    conway->cells = config;

    brian = init_automaton(200, &brians_brain, 2);
    brian->cells = config;

    langton = init_automaton(200, &langtons_ant, 2);
    struct ant *ant = init_ant(100, 100, 1, 1, langton);
    langton->cells = config;

    /* fps regulation */
    unsigned int frame_start;
    unsigned int frame_curr;
    unsigned int wait_time;

    /* main program execution */
    active = conway;
    frame_start = SDL_GetTicks();

    while (running) {

        handle_input();

        /* render automaton */
        render(active);

        if (!paused) {

            if (ticks > speed) {

                /* update automaton */
                active->sim(active);

                ticks = 0;
                gens++;

            } else {

                ticks++;

            }

        } else {
            if (step) {
                active->sim(active);
                gens++;
                step = !step;
            }
        }

        /* fix framerate at FRAME_RATE fps */
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

void switch_automaton(struct automaton *new) {

    new->cells = active->cells;
    bzero(new->cells, sizeof(int));
    active = new;

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
                case FIRING:
                    SDL_SetRenderDrawColor(renderer, 76, 242, 4, SDL_ALPHA_OPAQUE);
                    break;
                case DYING:
                    SDL_SetRenderDrawColor(renderer, 229, 0, 225, SDL_ALPHA_OPAQUE);
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

        if (ca->is_langton) {
            int offset = ca->ant->y * ca->len + ca->ant->x;
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
            SDL_RenderFillRect(renderer, &ca->rects[offset]);
        }

        SDL_SetRenderDrawColor(renderer, 16, 16, 16, SDL_ALPHA_OPAQUE);
        for (int i = 1; i < ca->len; i++) {
            int x = i * ca->cell_width;
            int y = i * ca->cell_height;

            SDL_RenderDrawLine(renderer, x, 0, x, WIN_HEIGHT);
            SDL_RenderDrawLine(renderer, 0, y, WIN_WIDTH, y);
        }

        /* only show generations for 2D automata */

    }

    SDL_RenderPresent(renderer);
}

void handle_input() {

    /* for mouse coords */
    static int x1, y1, x2, y2;

    static bool mouse = false;
    static bool ctrl = false;

    /* keep track of which cell to modify in DRAW mode */
    int n_i, n_j;

    /* to calculate total size of array for memset */
    int arr_size;

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
                mouse = true;
                n_i = e.motion.x / active->cell_width;
                n_j = e.motion.y / active->cell_height;

                active->cells[n_j*active->len + n_i] = paintbrush;
                break;
            case SDL_BUTTON_RIGHT:
                n_i = e.motion.x / active->cell_width;
                n_j = e.motion.y / active->cell_height;

                active->cells[n_j*active->len + n_i] = DEAD;
            }
            break;
        case SDL_MOUSEBUTTONUP:
            mouse = false;
            break;
        case SDL_KEYDOWN:
            switch (e.key.keysym.sym) {
            case SDLK_LSHIFT:
                SDL_GetMouseState(&x1, &y1);
                break;
            case SDLK_LCTRL:
                ctrl = true;
                break;
            case SDLK_SPACE:
                step = true;
                break;
            case SDLK_RIGHT:
                speed--;
                speed = speed < 0 ? 0 : speed;
                break;
            case SDLK_LEFT:
                speed++;
                break;
            case SDLK_c:
                arr_size = pow(active->len,active->dimension) * sizeof(int);
                printf("Clearing %d bytes\n", arr_size);
                memset(
                    active->cells,
                    DEAD,
                    arr_size
                );
                gens = 0;
                break;
            case SDLK_p:
                if (paused)
                    printf("Resuming...\n");
                else
                    printf("Paused\n");
                paused = !paused;
                break;
            case SDLK_q:
                running = 0;
                break;
            case SDLK_r:
                gens = 0;
                break;
            case SDLK_0:
                if (ctrl) {
                    active = rule110;
                } else {
                    paintbrush = ALIVE;
                }
                break;
            case SDLK_1:
                if (ctrl) {
                    switch_automaton(conway);
                    printf("Welcome to the Game of Life!\n");
                } else {
                    paintbrush = DEAD;
                }
                break;
            case SDLK_2:
                if (ctrl) {
                    switch_automaton(wires);
                    printf("Welcome to WireWorld!\n");
                } else {
                    paintbrush = ALIVE;
                }
                break;
            case SDLK_3:
                if (ctrl) {
                    switch_automaton(brian);
                    printf("Welcome to Brian's Brain!\n");
                } else {
                    paintbrush = HEAD;
                }
                break;
            case SDLK_4:
                if (ctrl) {
                    switch_automaton(langton);
                    printf("Welcome to Langton's Ant!\n");
                } else {
                    paintbrush = CONDUCTOR;
                }
                break;
            case SDLK_5:
                if (ctrl) {
                } else {
                    paintbrush = FIRING;
                }
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
            case SDLK_LCTRL:
                ctrl = false;
                break;
            }

        }

    }

    if (mouse) {
        SDL_GetMouseState(&x1, &y1);
        n_i = x1 / active->cell_width;
        n_j = y1 / active->cell_height;

        active->cells[n_j*active->len + n_i] = paintbrush;
    }

    const Uint8 *keystate = SDL_GetKeyboardState(NULL);

    /* continuous response keys */
    if (keystate[SDL_SCANCODE_LCTRL]) {
    }

    if(keystate[SDLK_RIGHT]) {
    }

    if(keystate[SDLK_UP]) {
    }

    if(keystate[SDLK_DOWN]) {
    }

}

/* linear interpolation to approximate a line on the grid */
float lerp(float start, float end, float t) {
    return start + t * (end - start);
}

/* calculate value for N when interpolating */
int diag(int x1, int y1, int x2, int y2) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    return max(abs(dx), abs(dy));
}
