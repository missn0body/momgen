CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c2x -g -Ofast
RM = rm

BIN = example
OBJS = $(BIN).o

.PHONY: all clean

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -f $(BIN) *.o
