#define DEBUG 
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>

int main (int argc, char* argv[] ) {
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
  int getopt_result;
  do {
    getopt_result = getopt_long(argc, argv, optstring, long_options, NULL);
    #ifdef DEBUG
    printf("%d %c %d\n", getopt_result, (char)getopt_result, optind);
    #endif
  } while (getopt_result != -1);
}