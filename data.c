/*
  Implementazione di una coda FIFO
*/
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
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
    new_node->data.i.length = random() % length + 1; // Va in errore se length == 0
  } else {
    new_node->data.i.length = length;
  }
  return new_node;
}
void callFunctionOnList(List_t* list, void (*function)(Node_t*)) {
  if (list->head == NULL)
    return;
  pthread_mutex_lock(&list->mutex);
  Node_t* p = list->head;
  Node_t* q = NULL;
  while(p != NULL) {
    q = p->next;
    function(p);
    p = q;
  }
  pthread_mutex_unlock(&list->mutex);
}
void printNode(Node_t* node) {
  if (node == NULL)
    return;
  if (node->data_type == 'i') {
    if (node->data.i.blocking)
      printf("I-B  L: %d\n", node->data.i.length);
    else
      printf("I-NB L: %d\n", node->data.i.length);
  } else if (node->data_type == 't') {
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
void push(List_t* list, Node_t* node) {
  if (list == NULL || node == NULL)
    return;
  pthread_mutex_lock(&list->mutex);
  if (list->head == NULL) {
    list->head = node;
    list->tail = node;
  } else {
    list->tail->next = node;
    list->tail = node;
  }
  pthread_mutex_unlock(&list->mutex);
}
void pop(List_t* list) {
  if (list == NULL || list->head == NULL)
    return;
  pthread_mutex_lock(&list->mutex);
  Node_t* nodeToDelete = list->head;
  list->head = list->head->next;
  destroyNode(nodeToDelete);
  if (list->head == NULL)
    list->tail = NULL;
  pthread_mutex_unlock(&list->mutex);
}