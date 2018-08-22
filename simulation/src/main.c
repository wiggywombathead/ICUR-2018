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

SDL_Surface *surface;
SDL_Texture *texture;
SDL_Rect msg_rect;

bool running = true;
bool paused = false;
bool step = false;
int paintbrush = ALIVE;     /* the state with which to overwrite cell */

/* simulation speed */
int speed = 0;
int gens = 0;

/* aesthetics */
bool gridlines = true;

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

int main(int argc, char *argv[]) {

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

    /* text setup */
    TTF_Font *font = TTF_OpenFont("consolas.ttf", 144);
    if(!font) {
        fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
    }

    SDL_Colour white = {255, 255, 255};
    msg_rect.w = 1 * TXT_WIDTH;
    msg_rect.h = 18;
    msg_rect.x = WIN_WIDTH - msg_rect.w;
    msg_rect.y = 0;

    char gen_str[12];
    int gen_len = 1;
    int last_gen_len = gen_len;

    srand(time(NULL));

    /* automata initialisation */
    int *config_1d = calloc(SIZE_1D, sizeof(int));
    config_1d[400] = 1;

    int *config = calloc(SIZE_2D*SIZE_2D, sizeof(int));

    rule30 = init_automaton(SIZE_1D, &rule_30, 1);
    rule30->cells = config_1d;

    rule54 = init_automaton(SIZE_1D, &rule_54, 1);
    rule54->cells = config_1d;

    rule90 = init_automaton(SIZE_1D, &rule_90, 1);
    rule90->cells = config_1d;

    rule110 = init_automaton(SIZE_1D, &rule_110, 1);
    rule110->cells = config_1d;
    
    rule150 = init_automaton(SIZE_1D, &rule_150, 1);
    rule150->cells = config_1d;

    rule182 = init_automaton(SIZE_1D, &rule_182, 1);
    rule182->cells = config_1d;

    rule232 = init_automaton(SIZE_1D, &rule_232, 1);
    rule232->cells = config_1d;

    rule250 = init_automaton(SIZE_1D, &rule_250, 1);
    rule250->cells = config_1d;

    wires = init_automaton(SIZE_2D, &wireworld, 2);
    wires->cells = config;

    conway = init_automaton(SIZE_2D, &game_of_life, 2);
    conway->cells = config;

    brian = init_automaton(SIZE_2D, &brians_brain, 2);
    brian->cells = config;

    langton = init_automaton(SIZE_2D, &langtons_ant, 2);
    struct ant *ant = init_ant(100, 100, 0, -1, langton);
    langton->cells = config;
    langton->ant = ant;

    /* fps regulation */
    unsigned int frame_start;
    unsigned int frame_curr;
    unsigned int wait_time;

    /* simulation speed */
    int ticks = 0;

    /* main program execution */
    if (argc == 2) {
        if (atoi(argv[1]) == 1) {
            active = rule30;
        } else {
            active = conway;
        }
    } else {
        active = conway;
    }

    frame_start = SDL_GetTicks();

    while (running) {

        handle_input();

        sprintf(gen_str, "%d", gens);
        if ((gen_len = strlen(gen_str)) > last_gen_len) {
            last_gen_len = gen_len;
            msg_rect.w += TXT_WIDTH;
            msg_rect.x -= TXT_WIDTH;
        }

        surface = TTF_RenderText_Solid(font, gen_str, white);
        texture = SDL_CreateTextureFromSurface(renderer, surface);

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

        /* perhaps inefficient, but it works (TODO) */
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);

        /* fix framerate at FRAME_RATE fps */
        frame_curr = SDL_GetTicks();

        if (frame_curr < frame_start + FRAME_INTERVAL) {
            wait_time = frame_start + FRAME_INTERVAL - frame_curr;
            SDL_Delay(wait_time);
        }

        frame_start = frame_curr;

    }

    TTF_CloseFont(font);
    TTF_Quit();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}

void switch_to_automaton(struct automaton *new) {

    new->cells = active->cells;
    bzero(new->cells, sizeof(int));
    active = new;

}

int switch_to_elem(int n) {
    
    switch (n) {
    case 30:
        active = rule30;
        break;
    case 54:
        active = rule54;
        break;
    case 90:
        active = rule90;
        break;
    case 110:
        active = rule110;
        break;
    case 150:
        active = rule150;
        break;
    case 182:
        active = rule182;
        break;
    case 232:
        active = rule232;
        break;
    case 250:
        active = rule250;
        break;
    default:
        printf("Elementary CA %d not yet supported\n", n);
        return 0;
    }

    for (int i = 0; i < active->len; i++) {
        active->cells[i] = 0;
        active->rects[i].y = 0;
    }

    active->cells[SIZE_1D / 2] = 1;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    return 1;

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

        if (gridlines) {
            SDL_SetRenderDrawColor(renderer, 8, 8, 8, SDL_ALPHA_OPAQUE);
            for (int i = 1; i < ca->len; i++) {
                int x = i * ca->cell_width;
                int y = i * ca->cell_height;

                SDL_RenderDrawLine(renderer, x, 0, x, WIN_HEIGHT);
                SDL_RenderDrawLine(renderer, 0, y, WIN_WIDTH, y);
            }
        }

        /* only show generations for 2D automata */
        SDL_RenderCopy(renderer, texture, NULL, &msg_rect); 
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
            case SDLK_e:
                printf("Enter the rule number of the elementary CA: ");
                int num;
                scanf("%d", &num);
                while (getchar() != '\n')
                    ;
                switch_to_elem(num);
                break;
            case SDLK_g:
                gridlines = !gridlines;
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
                    switch_to_automaton(conway);
                    printf("Welcome to the Game of Life!\n");
                } else {
                    paintbrush = DEAD;
                }
                break;
            case SDLK_2:
                if (ctrl) {
                    switch_to_automaton(wires);
                    printf("Welcome to WireWorld!\n");
                } else {
                    paintbrush = ALIVE;
                }
                break;
            case SDLK_3:
                if (ctrl) {
                    switch_to_automaton(brian);
                    printf("Welcome to Brian's Brain!\n");
                } else {
                    paintbrush = HEAD;
                }
                break;
            case SDLK_4:
                if (ctrl) {
                    switch_to_automaton(langton);
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
