#ifndef _GLOBAL_H
#define _GLOBAL_H

#define WIN_WIDTH 800
#define WIN_HEIGHT 800

#define FRAME_RATE 60

#define TXT_WIDTH 9

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
    ANT,

    /* von Neumann's */
    U,
    S,
    S0,
    S00,
    S000,
    S01,
    S1,
    S10,
    S11,

    C00,
    C01,
    C10,
    C11,

    Noe,
    Noq,
    Eoe,
    Eoq,
    Soe,
    Soq,
    Woe,
    Woq,

    Nse,
    Nsq,
    Ese,
    Esq,
    Sse,
    Ssq,
    Wse,
    Wsq
};

#endif
