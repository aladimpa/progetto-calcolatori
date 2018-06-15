#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "options.c"
#include "input.c"
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
  // Leggi le opzioni da riga di comando
  parse_options(argv, argc, &input_filename, &output_preemption_filename, &output_no_preemption_filename);
  // Leggi il file di input
  parse_input(simulation_data, input_filename);
  destroyList(simulation_data);
  // Il processo padre attende il figlio
  if (pid != 0)
    waitpid(pid, NULL, 0);
}