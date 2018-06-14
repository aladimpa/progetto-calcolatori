# Variabili
CC ?= gcc
CFLAGS ?= -Werror -Wfatal-errors -Wall -Wextra -pedantic \
	-Og -g -ggdb -fsanitize=address -fsanitize=leak -fsanitize=undefined -pthread \
	-D DEBUG=yes
# CFLAGS = -O3 -pthread
# Definizione dei phony
.PHONY: all clean test test1 test2 test3 test4 test5
# Target
options.o: options.c
	$(CC) $(CFLAGS) -c $< -o $@
data.o: data.c
	$(CC) $(CFLAGS) -c $< -o $@
input.o: input.c data.o
	$(CC) $(CFLAGS) -c $< -o $@
simulator: simulator.c options.o input.o data.o
	$(CC) $(CFLAGS) $< -o $@
# Phony
all: simulator
clean:
	rm -f *.o
	rm -f *.out
	rm -f simulator
	rm -f *.log
test: test1 test2 test3 test4 test5
test1: all
	./simulator -i input_files/01_tasks.csv -on output-1-np.log -op output-1-pr.log > output-1.log
test2: all
	./simulator -i input_files/02_tasks.csv -on output-2-np.log -op output-2-pr.log > output-2.log
test3: all
	./simulator -i input_files/03_tasks.csv -on output-3-np.log -op output-3-pr.log > output-3.log
test4: all
	./simulator -i input_files/04_tasks.csv -on output-4-np.log -op output-4-pr.log > output-4.log
test5: all
	./simulator -i input_files/05_tasks.csv -on output-5-np.log -op output-5-pr.log > output-5.log