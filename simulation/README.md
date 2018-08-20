# Cellular automata visualiser
As part of my project I also wrote this tool to visual various automata
discussed in the paper, and indeed most of the figures there come from various
versions of this. It is written in C using SDL.

## Features
Currently, it can simulate:

#### Elementary automata
* Rule 30
* Rule 54
* Rule 90
* Rule 110
* Rule 150
* Rule 182
* Rule 232
* Rule 250

#### 2D automata
* Game of Life
* WireWorld
* Brian's Brain
* Langton's Ant

## Controls
* `m` - toggle between draw and simulate mode
* `0-9` - switch the material you are drawing with:
  * 1 - dead
  * 2 - alive
  * 3 - electron head
  * 4 - conductor
  * 5 - firing neuron
* `LShift` - hold to draw a straight line from start to where you release
* `Space` - step the simulation along one generation when paused

* `Left/Right arrow keys` - decrease/increase speed
* `LCtrl + [0-9]` - switch between different automata

* `c` - clear the screen
* `r` - reset the generation counter
* `p` - pause/resume simulation
* `h` - bring up help
* `q` - quit

## Notes
SegFaults changing 1D automata to another 1D automaton more than once

## TODO
* von Neumann's automaton
* Nobili's automaton
* Codd's automaton
* Langton loops
