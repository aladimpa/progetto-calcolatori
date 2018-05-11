# Variabili
CC = gcc
CFLAGS = -Werror -Wfatal-errors -Wall -Wextra -pedantic \
	-Og -g -ggdb -fsanitize=address -fsanitize=leak -fsanitize=undefined -pthread \
	-D DEBUG=yes
# CFLAGS = -O3 -pthread
# Definizione dei phony
.PHONY: all clean
# Target
options.o: options.c
	$(CC) $(CFLAGS) -c $< -o $@
simulator: simulator.c options.o
	$(CC) $(CFLAGS) $< -o $@
# Phony
all: simulator
clean:
	rm -f *.o
	rm -f simulator