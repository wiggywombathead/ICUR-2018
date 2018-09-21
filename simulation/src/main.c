#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "global.h"
#include "automaton.h"

#define AUTOMATA 10
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

struct automaton *amoeboid;
struct automaton *annealer;
struct automaton *brian;
struct automaton *conway;
struct automaton *langton;
struct automaton *lwodeath;
struct automaton *morley;
struct automaton *replicate;
struct automaton *twotwo;
struct automaton *wires;

struct automaton *automata[10];
int current_ca = 0;

struct automaton *active;

void switch_to_automaton(struct automaton *);
int switch_to_elem(int);

void render(struct automaton *);
void handle_input(void);
void print_message(int);

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
    config_1d[SIZE_1D / 2] = 1;

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

    amoeboid = init_automaton(SIZE_2D, &diamoeba, 2);
    amoeboid->cells = config;

    annealer = init_automaton(SIZE_2D, &anneal, 2);
    annealer->cells = config;

    brian = init_automaton(SIZE_2D, &brians_brain, 2);
    brian->cells = config;

    conway = init_automaton(SIZE_2D, &game_of_life, 2);
    conway->cells = config;

    lwodeath = init_automaton(SIZE_2D, &life_wo_death, 2);
    lwodeath->cells = config;

    morley = init_automaton(SIZE_2D, &move, 2);
    morley->cells = config;

    replicate = init_automaton(SIZE_2D, &replicator, 2);
    replicate->cells = config;

    twotwo = init_automaton(SIZE_2D, &two_by_two, 2);
    twotwo->cells = config;

    wires = init_automaton(SIZE_2D, &wireworld, 2);
    wires->cells = config;

    langton = init_automaton(SIZE_2D, &langtons_ant, 2);
    struct ant *ant = init_ant(100, 100, 0, -1, langton);
    langton->cells = config;
    langton->ant = ant;

    automata[0] = conway;
    automata[1] = wires;
    automata[2] = brian;
    automata[3] = langton;
    automata[4] = amoeboid;
    automata[5] = replicate;
    automata[6] = morley;
    automata[7] = lwodeath;
    automata[8] = twotwo;
    automata[9] = annealer;

    /* fps regulation */
    unsigned int frame_start;
    unsigned int frame_curr;
    unsigned int wait_time;

    /* simulation speed */
    int ticks = 0;

    /* main program execution */
    // active = automata[0];
    active = rule30;

    frame_start = SDL_GetTicks();

    while (running) {

        // sprintf(gen_str, "%d", gens);
        // if ((gen_len = strlen(gen_str)) > last_gen_len) {
        //     last_gen_len = gen_len;
        //     msg_rect.w += TXT_WIDTH;
        //     msg_rect.x -= TXT_WIDTH;
        // }

        // surface = TTF_RenderText_Solid(font, gen_str, white);
        // texture = SDL_CreateTextureFromSurface(renderer, surface);

        /* do what it says on the tin */
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

        /* perhaps inefficient, but it works (TODO) */
        // SDL_DestroyTexture(texture);
        // SDL_FreeSurface(surface);

        /* fix framerate at FRAME_RATE fps */
        frame_curr = SDL_GetTicks();

        if (frame_curr < frame_start + FRAME_INTERVAL) {
            wait_time = frame_start + FRAME_INTERVAL - frame_curr;
            SDL_Delay(wait_time);
        }

        frame_start = frame_curr;

        SDL_Delay(60);
    }

    TTF_CloseFont(font);
    TTF_Quit();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}

int mod(int a, int b) {
    int r = a % b;
    return r < 0 ? r + b : r;
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
                    SDL_SetRenderDrawColor(renderer, 0, 100, 200, SDL_ALPHA_OPAQUE);
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

    static bool l_mouse = false;
    static bool r_mouse = false;
    static bool ctrl = false;

    /* keep track of which cell to modify in DRAW mode */
    int n_i, n_j;

    int index;

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
                l_mouse = true;
                n_i = e.motion.x / active->cell_width;

                if (active->dimension == 1) {
                    active->cells[n_i] = ALIVE;
                } else {
                    n_j = e.motion.y / active->cell_height;
                    active->cells[n_j*active->len + n_i] = paintbrush;
                }
                break;
            case SDL_BUTTON_RIGHT:
                r_mouse = true;
                n_i = e.motion.x / active->cell_width;

                if (active->dimension == 1) {
                    active->cells[n_i] = DEAD;
                } else {
                    n_j = e.motion.y / active->cell_height;
                    active->cells[n_j*active->len + n_i] = DEAD;
                }
            }
            break;
        case SDL_MOUSEBUTTONUP:
            l_mouse = r_mouse = false;
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
                reset_automaton(active);
                if (active->dimension == 1) {
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
                    SDL_RenderClear(renderer);
                    SDL_RenderPresent(renderer);
                }
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
            case SDLK_m:
                index = mod(--current_ca, AUTOMATA);
                switch_to_automaton(automata[index]);
                print_message(index);
                break;
            case SDLK_n:
                index = mod(++current_ca, AUTOMATA);
                switch_to_automaton(automata[index]);
                print_message(index);
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
                    print_message(0);
                } else {
                    paintbrush = DEAD;
                }
                break;
            case SDLK_2:
                if (ctrl) {
                    switch_to_automaton(wires);
                    print_message(1);
                } else {
                    paintbrush = ALIVE;
                }
                break;
            case SDLK_3:
                if (ctrl) {
                    switch_to_automaton(brian);
                    print_message(2);
                } else {
                    paintbrush = HEAD;
                }
                break;
            case SDLK_4:
                if (ctrl) {
                    switch_to_automaton(langton);
                    print_message(3);
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

    if (l_mouse) {
        SDL_GetMouseState(&x1, &y1);
        n_i = x1 / active->cell_width;
        n_j = y1 / active->cell_height;

        if (active->dimension == 1) {
            active->cells[n_i] = ALIVE;
        } else {
            active->cells[n_j*active->len + n_i] = paintbrush;
        }
    } else if (r_mouse) {
        SDL_GetMouseState(&x1, &y1);
        n_i = x1 / active->cell_width;
        n_j = y1 / active->cell_height;

        if (active->dimension == 1) {
            active->cells[n_i] = DEAD;
        } else {
            active->cells[n_j*active->len + n_i] = paintbrush;
        }
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

void print_message(int n) {

    char msg[64] = "Welcome to ";
    switch (n) {
    case 0:
        strcat(msg, "Game of Life");
        break;
    case 1:
        strcat(msg, "WireWorld");
        break;
    case 2:
        strcat(msg, "Brian's Brain");
        break;
    case 3:
        strcat(msg, "Langton's Ant");
        break;
    case 4:
        strcat(msg, "Diamoeboid");
        break;
    case 5:
        strcat(msg, "Replicator");
        break;
    case 6:
        strcat(msg, "Morley's automaton");
        break;
    case 7:
        strcat(msg, "Life without Death");
        break;
    case 8:
        strcat(msg, "Two By Two");
        break;
    case 9:
        strcat(msg, "Anneal");
        break;
    }
    strcat(msg, "!\n");
    printf("%s", msg);

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
