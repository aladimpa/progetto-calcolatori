#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "data.c"
void parse_input(List_t* list, char* filename) {
  // Apri il file in ingresso
  FILE* input_file = fopen(filename, "r");
  if (input_file == NULL) {
    perror("Errore nell'apertura del file");
    exit(1);
  }
  pthread_cleanup_push(closeFile, input_file);
  // Leggiamo il file
  char tipo;
  int parametro_1, parametro_2, match;
  Node_t* current_task = NULL;
  while((match = fscanf(input_file, "%c,%d,%d\n", &tipo, &parametro_1, &parametro_2)) != EOF){
    if (match != 3) {
      fprintf(stderr,"%s\n","File di input malformato");
      exit(1);
    }
    // Parsing delle righe
    switch (tipo) {
      case 't':
        // Si tratta di un task
        if (current_task != NULL)
          push(list, current_task);
        current_task = newTaskNode(parametro_1, parametro_2);
        break;
      case 'i':
        // Si tratta di una istruzione
        if (current_task == NULL) {
          fprintf(stderr,"%s\n","File di input malformato");
          exit(1);
        }
        Node_t* current_instruction;
        switch (parametro_1) {
          case 0:
            // Istruzione non bloccante
            current_instruction = newInstructionNode(false, parametro_2);
            break;
          case 1:
            // Istruzione bloccante
            current_instruction = newInstructionNode(true, parametro_2);
            break;
          default:
            fprintf(stderr,"%s\n","File di input malformato");
            exit(1);
            break;
        }
        push(current_task->data.t.instructions, current_instruction);
        break;
      default:
        fprintf(stderr,"%s\n","File di input malformato");
        exit(1);
        break;
    }
  }
  if (current_task != NULL)
    push(list, current_task);
  // Chiudiamo il file
  pthread_cleanup_pop(1);
}