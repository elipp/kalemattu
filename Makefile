IDIR =include
CC=clang
CFLAGS=-I$(IDIR) -Wall -pedantic -g -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE 

ODIR=obj

SRCDIR=src

LIBS=-lm -lpthread -lircclient
DEPS=include/types.h

_OBJ = irc.o aesthetics.o distributions.o dict.o poem.o stringutil.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

kalemattuc: $(OBJ) $(SRCDIR)/kalemattu.c
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
