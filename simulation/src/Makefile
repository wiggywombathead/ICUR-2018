CC = gcc
IDIR = ../include
ODIR = objs
LIBS = -lSDL2 -lSDL2_ttf -lm -pthread `sdl-config --libs`
CFLAGS = -O2 -Wall -g -I$(IDIR) `sdl-config --cflags`

_DEPS = automaton.h global.h
DEPS = $(patsubst %, $(IDIR)/%, $(_DEPS))

_OBJ = automaton.o main.o
OBJ = $(patsubst %, $(ODIR)/%, $(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	mkdir -p $(@D)
	$(CC) -c -o $@ $< $(CFLAGS)

run: $(OBJ)
	mkdir -p $(@D)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -rf $(ODIR)
