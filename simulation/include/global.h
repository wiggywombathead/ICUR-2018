#ifndef _GLOBAL_H
#define _GLOBAL_H

#define WIN_WIDTH 512
#define WIN_HEIGHT 512

#define FRAME_RATE 60

enum states {

    /* Game of Life */
    DEAD = 0,
    ALIVE = 1,

    /* BRIAN'S BRAIN */
    FIRING,
    DYING,
    
    /* WireWorld */
    HEAD,
    TAIL,
    CONDUCTOR,

    /* Langton's Ant */
    ANT
};

#endif
