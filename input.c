#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

void parse_input(char* filename) {
  // Apri il file in ingresso
  int input_file_descriptor = open(filename, O_RDONLY);
  if (input_file_descriptor == -1) {
    perror("Errore nella lettura del file di input");
    exit(1);
  }  //non c'è l'else perchè se entro nell'if poi esco con exit
  // Diciamo al sistema operativo che
  // POSIX_FADV_SEQUENTIAL
  //   Leggeremo il file dall'inizio alla fine
  // POSIX_FADV_NOREUSE
  //   Leggeremo il file una volta sola
  posix_fadvise(input_file_descriptor, 0, 0, POSIX_FADV_SEQUENTIAL | POSIX_FADV_NOREUSE);
  // Vado a prendere la lunghezza del file di input
  struct stat input_file_stat;
  if (fstat(input_file_descriptor, &input_file_stat) == -1) {
    perror("Errore nella lettura del file di input");
    exit(1);
  }
  //conversione dimensione da off_t a size_t
  size_t input_file_size = (size_t) input_file_stat.st_size;
  #ifdef DEBUG
  printf("Dimensione dell'input: %ld byte\n", input_file_size);
  #endif
  // Mappato il file in memoria
  char* input_file = (char*) mmap(NULL, input_file_size, PROT_READ, MAP_SHARED, input_file_descriptor, 0);
  if ( input_file == MAP_FAILED ) {
    perror("Errore nella lettura del file di input");
    #ifdef DEBUG
    fprintf(stderr,"mmap fallito\n");
    #endif
    exit(1);
  }
  // Rimozione del file mappato dalla memoria
  if ( munmap(input_file, input_file_size) == -1 ) {
    perror("Errore nella rimozione del file di input dalla memoria");
    exit(1);
  }
}