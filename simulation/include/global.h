#ifndef _GLOBAL_H
#define _GLOBAL_H

#define WIN_WIDTH 512
#define WIN_HEIGHT 512

#define FRAME_RATE 60

enum states {
    /* GoL */
    DEAD = 0,
    ALIVE = 1,
    
    /* WireWorld */
    EMPTY,
    HEAD,
    TAIL,
    CONDUCTOR,

    /* Langton's Ant */
    ANT
};

enum modes {
    SIMULATE,
    DRAW
};

#endif
