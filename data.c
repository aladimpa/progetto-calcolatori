/*
  Implementazione di una coda FIFO
*/
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
// Per includere due volte devo fare in modo che venga ignorato la seconda volta
// Se non è definito data, posso includer
#ifndef DATA
#define DATA

#define PRINT_TASKS 1
#define PRINT_INSTRUCTIONS 0
// Definizione dei tipi di dato utilizzati
typedef enum ProcessState {
  NEW,
  READY,
  RUNNING,
  BLOCKED,
  EXIT
} ProcessState_t;
typedef struct Instruction Instruction_t;
typedef struct Task Task_t;
typedef struct List List_t;
typedef struct Node Node_t;

struct Instruction {
  bool blocking;
  int length;
};
struct Task {
  int id;
  Node_t* program_counter;
  int instruction_progress;
  int running_on;
  int arrival_time;
  List_t* instructions;
  ProcessState_t state;
};
typedef union ListContent {
  Instruction_t i;
  Task_t t;
} ListContent_t;
struct List {
  Node_t* head;
  Node_t* tail;
  pthread_mutex_t mutex;
};
struct Node {
  Node_t* next;
  ListContent_t data;
  char data_type;
};
// Metodi per la lista
List_t* newList() {
  List_t* new_list = (List_t*) malloc (sizeof(List_t));
  if (new_list == NULL) {
    fprintf(stderr,"%s\n","Allocazione di memoria per la lista fallita");
    exit(1);
  }
  new_list->head = NULL;
  new_list->tail = NULL;
  pthread_mutex_init(&new_list->mutex, NULL);
  return new_list;
}
Node_t* newTaskNode(int id, int arrival_time) {
  Node_t* new_node = (Node_t*) malloc (sizeof(Node_t));
  if (new_node == NULL) {
    fprintf(stderr,"%s\n","Allocazione di memoria per un nodo fallita");
    exit(1);
  }
  new_node->data_type = 't';
  new_node->next = NULL;
  new_node->data.t.id = id;
  new_node->data.t.arrival_time = arrival_time;
  new_node->data.t.instruction_progress = 0;
  new_node->data.t.running_on = 0;
  new_node->data.t.program_counter = NULL;
  new_node->data.t.state = NEW;
  new_node->data.t.instructions = newList();
  new_node->data.t.instructions->head = NULL;
  new_node->data.t.instructions->tail = NULL;
  return new_node;
}
Node_t* newInstructionNode(bool blocking, int length) {
  // Se bloccante aggiungere random
  Node_t* new_node = (Node_t*) malloc (sizeof(Node_t));
  if (new_node == NULL) {
    fprintf(stderr,"%s\n","Allocazione di memoria per un nodo fallita");
    exit(1);
  }
  new_node->data_type = 'i';
  new_node->next = NULL;
  new_node->data.i.blocking = blocking;
  if (blocking) {
    srandom(time(NULL));
    new_node->data.i.length = random() % length + 1; // Va in errore se length == 0
  } else {
    new_node->data.i.length = length;
  }
  return new_node;
}
void callFunctionOnList(List_t* list, void (*function)(Node_t*)) {
  if (list->head == NULL)
    return;
  Node_t* p = list->head;
  Node_t* q = NULL;
  while(p != NULL) {
    q = p->next;
    function(p);
    p = q;
  }
}
void printNode(Node_t* node) {
  if (node == NULL)
    return;
  #if PRINT_INSTRUCTIONS
  if (node->data_type == 'i') {
    if (node->data.i.blocking)
      printf("I-B  L: %d\n", node->data.i.length);
    else
      printf("I-NB L: %d\n", node->data.i.length);
  }
  #endif
  #if PRINT_TASKS
  if (node->data_type == 't') {
    switch (node->data.t.state) {
      case NEW:
        printf("Task  ID: %d T-A: %d NEW\n", node->data.t.id, node->data.t.arrival_time);
        break;
      case READY:
        printf("Task  ID: %d T-A: %d READY\n", node->data.t.id, node->data.t.arrival_time);
        break;
      case RUNNING:
        printf("Task  ID: %d T-A: %d RUN\n", node->data.t.id, node->data.t.arrival_time);
        break;
      case BLOCKED:
        printf("Task  ID: %d T-A: %d BLOCKED\n", node->data.t.id, node->data.t.arrival_time);
        break;
      case EXIT:
        printf("Task  ID: %d T-A: %d EXIT\n", node->data.t.id, node->data.t.arrival_time);
        break;
    }
    callFunctionOnList(node->data.t.instructions, &printNode);
  }
  #endif
}
void printList(List_t* list) {
  callFunctionOnList(list, &printNode);
}
void destroyNode(Node_t* node) {
  if (node == NULL)
    return;
  if (node->data_type == 'i') {
    free(node);
  } else if (node->data_type == 't') {
    callFunctionOnList(node->data.t.instructions, &destroyNode);
    free(node->data.t.instructions);
    free(node);
  }
}
void destroyList(List_t* list) {
  if (list == NULL)
    return;
  // Cancellazione dei nodi
  callFunctionOnList(list, &destroyNode);
  pthread_mutex_destroy(&list->mutex);
  free(list);
}
void destroyListOnExit(int status, void* list) { // Wrapper per destroyList e usarla da on_exit
  if (status != 0)
    fprintf(stderr, "%s\n", "Programma terminato con errore, pulizia lista in corso");
  destroyList((List_t*) list);
}
void destroyListFromThread(void* list) { // Wrapper per destroyList e usarla da pthread_cleanup_push
  destroyList((List_t*) list);
}
void push(List_t* list, Node_t* node) {
  if (list == NULL || node == NULL)
    return;
  node->next = NULL;
  // Lista vuota
  if (list->head == NULL) {
    list->head = node;
    list->tail = node;
    return;
  }
  // Lista non vuota
  list->tail->next = node;
  list->tail = node;
}
Node_t* popAt(List_t* list, Node_t* node) {
  if (list == NULL || list->head == NULL)
    return NULL;
  // Variabili
  Node_t* current_node = list->head;
  Node_t* popped_node = NULL;
  // Controllo se il nodo cercato é in testa
  if (node == current_node) {
    popped_node = list->head;
    list->head = list->head->next;
    if (list->head == NULL)
      list->tail = NULL;
    return popped_node;
  }
  // Cerco il nodo nella lista
  while (current_node != NULL) {
    if (current_node->next == node) {
      popped_node = current_node->next;
      if (popped_node != NULL)
        current_node->next = popped_node->next;
      else
        current_node->next = NULL;
      break; // Esco dal while
    }
    current_node = current_node->next;
  }
  if (list->head != NULL && list->head->next == NULL)
    list->tail = list->head;
  popped_node->next = NULL;
  return popped_node;
}
void cancelAt(List_t* list, Node_t* node) {
  destroyNode(popAt(list, node));
}
#endif