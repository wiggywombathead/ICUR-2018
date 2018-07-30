CC = gcc
IDIR = ../include
ODIR = objs
LIBS = -lSDL2
CFLAGS = -O2 -Wall -g -I$(IDIR)

_DEPS = automaton.h global.h
DEPS = $(patsubst %, $(IDIR)/%, $(_DEPS))

_OBJ = automaton.o main.o
OBJ = $(patsubst %, $(ODIR)/%, $(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

run: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -rf $(ODIR)
