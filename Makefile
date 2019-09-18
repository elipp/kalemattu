IDIR =include
CC=gcc
CFLAGS=-I$(IDIR) -Wall -pedantic -g -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE 

ODIR=obj

SRCDIR=src

LIBS=-lm -lpthread -lircclient -lfcgi
DEPS=include/types.h
REV=$(shell git rev-parse --short HEAD)
COMMITDATE=$(shell git log -1 --date=format:'%d\\/%m\\/%Y' --format=%cd)

_OBJ = irc.o aesthetics.o distributions.o dict.o poem.o stringutil.o fcgi.o synth.o han.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

kalemattuc: $(OBJ) $(SRCDIR)/kalemattu.c
	sed -i "s/<aside>\(.*\)<\/aside>/<aside>git revision: $(REV) ($(COMMITDATE))<\/aside>/g" index.html
	gcc -DREVISION=$(REV) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
