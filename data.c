// Implementazione di una lista linkata semplice
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef DATA
#define DATA
// Definizioni
// Nodo
typedef struct node node_t;
struct node {
  void* data;
  node_t* next;
};
// Lista
typedef struct list {
  node_t* head;
  node_t* tail;
} list_t;
// Task
typedef struct task {
  int id;
  int arrival_time;
  list_t instructions;
} task_t;
// Istruzioni
typedef struct instruction {
  bool blocking;
  int length;
} instruction_t;
// Metodi
list_t* create_list() {
  list_t* new_list = (list_t*) malloc(sizeof(list_t));
  new_list->head = NULL;
  new_list->tail = NULL;
  return new_list;
}
#endif