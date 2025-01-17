#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "options.c"
#include "input.c"
#include "scheduler.c"
int main (int argc, char* argv[] ) {
  // Fork
  pid_t pid = fork();
  if (pid == -1) {
    printf("%s\n","Fork fallita");
    exit(1);
  }
  #ifdef DEBUG
  if (pid == 0)
    printf("Sono il figlio\n");
  else
    printf("Sono il padre\n");
  #endif
  // variabili di stato
  char* input_filename = NULL;
  char* output_preemption_filename = NULL;
  char* output_no_preemption_filename = NULL;
  List_t* simulation_data = newList();
  on_exit(&destroyListOnExit, simulation_data);
  // Leggi le opzioni da riga di comando
  parse_options(argv, argc, &input_filename, &output_preemption_filename, &output_no_preemption_filename);
  // Leggi il file di input
  parse_input(simulation_data, input_filename);
  // Effettua la simulazione
  if (pid == 0)
    schedulate(simulation_data, output_preemption_filename, PREEMPTIVE);
  else
    schedulate(simulation_data, output_no_preemption_filename, NONPREEMPTIVE);
  // Il processo padre attende il figlio
  if (pid != 0)
    waitpid(pid, NULL, 0);
}