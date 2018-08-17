# Cellular automata visualiser
As part of my project I also wrote this tool to visual various automata
discussed in the paper, and indeed most of the figures there come from various
versions of this. It is written in C using SDL.

## Features
Currently, it can simulate:
* Rule 90
* Rule 110
* Game of Life
* WireWorld

## Controls
`m` - toggle between draw and simulate mode
`0-9` - switch the material you are drawing with:
  * 0 - dead/empty
  * 1 - alive
  * 2 - electron head
  * 3 - conductor
`LShift` - hold to draw a straight line from start to where you release

`h` - bring up help
`p` - pause/resume simulation
`q` - quit

## Notes
Only switch between different 2D cellular automata for now

## TODO
* Langton's Ant (in progress)
* Brian's Brain
