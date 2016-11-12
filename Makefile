IDIR=include
CC=gcc

#CFLAGS=-I$(IDIR)
CFLAGS=-Wall
OBJDIR=obj
CLIENT_ODIR=$(OBJDIR)/client
SERVER_ODIR=$(OBJDIR)/server
LDIR=lib

# Required for strlcat() and strlcpy() functions
CLIENT_LIBS=-lbsd

# POSIX Threads support
SERVER_LIBS=-lpthread

DELIVERABLES=fimerclient fimerd

DEBUG=no

ifeq ($(DEBUG),yes)
	CFLAGS += -ggdb3
else
	CFLAGS += -O2
endif

########################################################
# Client build
########################################################
C_DEPS = opcodes.h
CLIENT_DEPS = $(patsubst %,$(IDIR)/%,$(C_DEPS))

C_OBJ = fimerclient.o
CLIENT_OBJ = $(patsubst %,$(CLIENT_ODIR)/%,$(C_OBJ))

$(CLIENT_ODIR)/%.o: %.c $(CLIENT_DEPS)
	@mkdir -p $(@D)
	$(CC) -c -o $@ $< $(CFLAGS)
########################################################
# Server build
########################################################
S_DEPS = opcodes.h job.h linkedlist.h utilities.h parser.h \
		 operations.h
SERVERD_DEPS = $(patsubst %,$(IDIR)/%,$(S_DEPS))

SD_OBJ = fimerd.o linkedlist.o parser.o operations.o
SERVERD_OBJ = $(patsubst %,$(SERVER_ODIR)/%,$(SD_OBJ))
	
$(SERVER_ODIR)/%.o: %.c $(SERVERD_DEPS)
	@mkdir -p $(@D)
	$(CC) -c -o $@ $< $(CFLAGS)

########################################################
# Binary builds
########################################################
fimerclient: $(CLIENT_OBJ)
	gcc -o $@ $^ $(CFLAGS) $(CLIENT_LIBS)

fimerd: $(SERVERD_OBJ)
	gcc -o $@ $^ $(CFLAGS) $(SERVER_LIBS)

all: $(DELIVERABLES)

.PHONY: clean

clean:
	rm -f $(CLIENT_ODIR)/*.o
	rm -f $(SERVER_ODIR)/*.o
	rm -f $(DELIVERABLES)
	rm -rf $(OBJDIR)
