IDIR=include
CC=gcc

#CFLAGS=-I$(IDIR)
CFLAGS=-g

ODIR=obj
LDIR=lib

# Required for strlcat() and strlcpy() functions
CLIENT_LIBS=-lbsd

# POSIX Threads support
SERVER_LIBS=-lpthread

DELIVERABLES=fimerclient fimerserver fimerd

#_DEPS = customheaderoffuture.h
#DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

#_OBJ = hellomake.o hellofunc.o 
#OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

C_OBJ = fimerclient.o
CLIENT_OBJ = $(patsubst %,$(ODIR)/%,$(C_OBJ))

S_OBJ = fimerserver.o
SERVER_OBJ = $(patsubst %,$(ODIR)/%,$(S_OBJ))

SD_OBJ = fimerd.o
SERVERD_OBJ = $(patsubst %,$(ODIR)/%,$(SD_OBJ))

$(ODIR)/%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

#$(ODIR)/%.o: %.c $(DEPS)
#	$(CC) -c -o $@ $< $(CFLAGS)

#hellomake: $(OBJ)
#	gcc -o $@ $^ $(CFLAGS) $(LIBS)

fimerclient: $(CLIENT_OBJ)
	gcc -o $@ $^ $(CFLAGS) $(CLIENT_LIBS)

fimerserver: $(SERVER_OBJ)
	gcc -o $@ $^ $(CFLAGS)

fimerd: $(SERVERD_OBJ)
	gcc -o $@ $^ $(CFLAGS) $(SERVER_LIBS)

all: $(DELIVERABLES)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
	rm -f $(DELIVERABLES)
