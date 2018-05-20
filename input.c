#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
void parse_input(char* filename) {
  // Apri il file in ingresso
  FILE* input_file = fopen(filename, "r");
  if (input_file == NULL) {
    perror("Errore nell'apertura del file");
    exit(1);
  }
  // Leggiamo il file
  char tipo;
  int parametro_1, parametro_2, match;
  while((match = fscanf(input_file, "%c,%d,%d\n", &tipo, &parametro_1, &parametro_2)) != EOF){
    if (match != 3) {
      fprintf(stderr,"%s\n","File di input malformato");
      exit(1);
    }
    // Parsing delle righe
    switch (tipo) {
      case 't':
        // Si tratta di un task

        break;
      case 'i':
        // Si tratta di una istruzione
        switch (parametro_1) {
          case 0:
            // Istruzione non bloccante
            break;
          case 1:
            // Istruzione bloccante
            break;
          default:
            fprintf(stderr,"%s\n","File di input malformato");
            exit(1);
            break;
        }
        break;
      default:
        fprintf(stderr,"%s\n","File di input malformato");
        exit(1);
        break;
    }
  }
  // Chiudiamo il file
  fclose(input_file);
}