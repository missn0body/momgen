CC = g++
CFLAGS = -Wall -Wextra -Wpedantic -std=c++23 -g -Ofast
RM = rm

BIN = example
OBJS = $(BIN).o

.PHONY: all clean

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -f $(BIN) *.o
