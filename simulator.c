#include <stdio.h>
#include <stdlib.h>
#include "options.c"
#include "input.c"
int main (int argc, char* argv[] ) {
  // variabili di stato
  char* input_filename = NULL;
  char* output_preemption_filename = NULL;
  char* output_no_preemption_filename = NULL;
  // Leggi le opzioni da riga di comando
  parse_options(argv, argc, &input_filename, &output_preemption_filename, &output_no_preemption_filename);
  // Leggi il file di input
  parse_input(input_filename);
}