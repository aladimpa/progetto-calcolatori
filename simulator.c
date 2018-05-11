#include <stdio.h>
#include <stdlib.h>
#include "options.c"
int main (int argc, char* argv[] ) {
  // variabili di stato
  char* input_filename = NULL;
  char* output_preemption_filename = NULL;
  char* output_no_preemption_filename = NULL;
  parse_options(argv, argc, input_filename, output_preemption_filename, output_no_preemption_filename);
}