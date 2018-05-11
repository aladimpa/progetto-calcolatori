#define DEBUG
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

void show_help(char* programname) {
  printf("%s %s %s\n%s\n%s\n%s\n%s\n",
    "Utilizzo:", programname,"[OPZIONI] ...",
    "-op | --output-preemption <FILE>     file con i risultati dello scheduler preemptive",
    "-on | --output-no-preemption <FILE>  file con i risultati dello scheduler non preemptive",
    "-i  | --input <FILE>                 file con i dati del simulatore",
    "-h  | --help                         mostra questo messaggio"
  );
  exit(0);
}

int main (int argc, char* argv[] ) {
  // variabili di stato
  char* input_filename = NULL;
  char* output_preemption_filename = NULL;
  char* output_no_preemption_filename = NULL;
  // inizio impostazione di getopt
  #ifdef DEBUG
  for (int i=0; i<argc; i++)
    printf("Argv[%d]: %s\n", i, argv[i]);
  #endif
  const struct option long_options[] = {
    {"output-preemption", 1, NULL, 1 },
    {"output-no-preemption", 1, NULL, 2},
    {"input", 1, NULL, 3},
    {"help", 0, NULL, 4},
    {NULL, 0, NULL, 0}
  };
  const char* optstring = "o:i:h";
  //parsing delle opzioni
  opterr = 0;
  int getopt_result;
  do {
    getopt_result = getopt_long(argc, argv, optstring, long_options, NULL);
    #ifdef DEBUG
    printf("%d %c %d %s\n", getopt_result, (char)getopt_result, optind, optarg);
    #endif
    if (getopt_result == -1)
      break;
    switch (getopt_result)
    {
      case 1:
        output_preemption_filename = optarg;
        break;
      case 2:
        output_no_preemption_filename = optarg;
        break;
      case 'o':
        if (strlen(optarg) != 1 || optind == argc || *argv[optind] == '-')
          show_help(argv[0]);
        if (*optarg == 'p') {
          output_preemption_filename = argv[optind];
        } else if (*optarg == 'n') {
          output_no_preemption_filename = argv[optind];
        } else {
          show_help(argv[0]);
        }
        optind++;
        break;
      case 3:
      case 'i':
        input_filename = optarg;
        break;
      case 4:
      case 'h':
        show_help(argv[0]);
        break;
      default:
        fprintf(stderr, "%s\n", "Opzione non riconosciuta");
        show_help(argv[0]);
        break;
    }
  } while (getopt_result != -1);
  // Mostra i file selezionati
  if (input_filename == NULL || output_preemption_filename == NULL || output_no_preemption_filename == NULL) {
    fprintf(stderr, "%s\n", "Non sono stati inseriti tutti i file richiesti");
    show_help(argv[0]);
  }
  #ifdef DEBUG
  printf("%s %s\n%s %s\n%s %s\n",
    "Input:                ", input_filename,
    "Output preemptive:    ", output_preemption_filename,
    "Output non preemptive:", output_no_preemption_filename
  );
  #endif
}