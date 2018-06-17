# Variabili
CC ?= gcc
CFLAGS ?= -Werror -Wfatal-errors -Wall -Wextra -pedantic \
	-Og -g -ggdb -pthread \
	-D DEBUG=yes
# -fsanitize=address -fsanitize=leak -fsanitize=undefined
# CFLAGS = -O3 -pthread
VALGRIND_OPTS = --read-var-info=yes \
	--error-exitcode=1 \
	--read-var-info=yes \
	--fair-sched=try \
	-v
# Definizione dei phony
.PHONY: all clean valgrind test1 test2 test3 test4 test5
# Target
options.o: options.c
	$(CC) $(CFLAGS) -c $< -o $@
data.o: data.c
	$(CC) $(CFLAGS) -c $< -o $@
input.o: input.c data.o
	$(CC) $(CFLAGS) -c $< -o $@
scheduler.o: scheduler.c data.o
	$(CC) $(CFLAGS) -c $< -o $@
simulator: simulator.c options.o input.o data.o scheduler.o
	$(CC) $(CFLAGS) $< -o $@
# Phony
all: simulator
clean:
	rm -f *.o
	rm -f *.out
	rm -f simulator
	rm -f *.log
	rm -f vgcore.*
valgrind: all
	# valgrind --tool=memcheck $(VALGRIND_OPTS) --leak-check=full --track-origins=yes ./simulator -i input_files/01_tasks.csv -on output-1-np.log -op output-1-pr.log
	valgrind --tool=helgrind $(VALGRIND_OPTS) --free-is-write=yes ./simulator -i input_files/01_tasks.csv -on output-1-np.log -op output-1-pr.log
test1: all
	./simulator -i input_files/01_tasks.csv -on output-1-np.log -op output-1-pr.log
test2: all
	./simulator -i input_files/02_tasks.csv -on output-2-np.log -op output-2-pr.log
test3: all
	./simulator -i input_files/03_tasks.csv -on output-3-np.log -op output-3-pr.log
test4: all
	./simulator -i input_files/04_tasks.csv -on output-4-np.log -op output-4-pr.log
test5: all
	./simulator -i input_files/05_tasks.csv -on output-5-np.log -op output-5-pr.log