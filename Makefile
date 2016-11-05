IDIR=include
CC=gcc

#CFLAGS=-I$(IDIR)
CFLAGS=-g

ODIR=obj
LDIR=lib

# Required for strlcat() and strlcpy() functions
LIBS=-lbsd

DELIVERABLES=fimerclient fimerserver

#_DEPS = customheaderoffuture.h
#DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

#_OBJ = hellomake.o hellofunc.o 
#OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

C_OBJ = fimerclient.o
CLIENT_OBJ = $(patsubst %,$(ODIR)/%,$(C_OBJ))

S_OBJ = fimerserver.o
SERVER_OBJ = $(patsubst %,$(ODIR)/%,$(S_OBJ))

$(ODIR)/%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

#$(ODIR)/%.o: %.c $(DEPS)
#	$(CC) -c -o $@ $< $(CFLAGS)

#hellomake: $(OBJ)
#	gcc -o $@ $^ $(CFLAGS) $(LIBS)

fimerclient: $(CLIENT_OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

fimerserver: $(SERVER_OBJ)
	gcc -o $@ $^ $(CFLAGS)

all: $(DELIVERABLES)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
	rm -f $(DELIVERABLES)
