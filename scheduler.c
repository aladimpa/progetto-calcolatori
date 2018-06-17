#include "data.c"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef enum SchedulerType {
  PREEMPTIVE,
  NONPREEMPTIVE
} SchedulerType_t;

typedef struct SchedulerInfo {
  int coreNumber;
  SchedulerType_t scheduler_type;
  List_t* simulation_data;
  pthread_mutex_t* simulation_data_mutex;
} SchedulerInfo_t;

void* schedule(void* sched_info) {
  // Prendo i parametri dello scheduler
  int coreNumber = ((SchedulerInfo_t*) sched_info)->coreNumber;
  #ifdef DEBUG
  printf("%s %d %s\n","Core number", coreNumber, "online");
  #endif
  // SchedulerType_t scheduler_type = ((SchedulerInfo_t*) sched_info)->scheduler_type;
  List_t* simulation_data = ((SchedulerInfo_t*) sched_info)->simulation_data;
  // Variabili di stato
  int clock = 1;
  bool executing_run = false;
  while (true) {
    if (simulation_data->head == NULL)
      break;
    pthread_mutex_lock(((SchedulerInfo_t*) sched_info)->simulation_data_mutex);
    Node_t* current_node = simulation_data->head;
    bool executing_blocking = false;
    while(current_node != NULL) {
      if (clock < current_node->data.t.arrival_time) {
        current_node = current_node->next;
        continue;
      }
      // Spostamento dei NEW in READY
      if (current_node->data.t.state == NEW) {
        printf("core%d,%d,%d,%s\n", coreNumber, clock, current_node->data.t.id, "NEW");
        current_node->data.t.program_counter = current_node->data.t.instructions->head;
        current_node->data.t.state = READY;
        printf("core%d,%d,%d,%s\n", coreNumber, clock, current_node->data.t.id, "READY");
      }
      // Esecuzione di uno in RUN
      if (current_node->data.t.state == READY && !executing_blocking && !executing_run) {
        // Ho a disposizione una istruzione
        if (current_node->data.t.program_counter->data.i.blocking) {
          current_node->data.t.state = BLOCKED;
          printf("core%d,%d,%d,%s\n", coreNumber, clock, current_node->data.t.id, "BLOCKED");
          executing_blocking = true;
        } else {
          current_node->data.t.state = RUNNING;
          printf("core%d,%d,%d,%s\n", coreNumber, clock, current_node->data.t.id, "RUN");
          executing_run = true;
        }
        current_node->data.t.running_on = coreNumber;
      }
      // Gestione dei RUN / BLOCKED
      bool repeat_analysis = false;
      if (current_node->data.t.running_on == coreNumber && (current_node->data.t.state == BLOCKED || current_node->data.t.state == RUNNING)) {
        // Va avanti nell'istruzione corrente
        current_node->data.t.instruction_progress++;
        // Controllo se l'esecuzione è terminata
        if (current_node->data.t.program_counter->data.i.length + 1 == current_node->data.t.instruction_progress) {
          // Avanza il program counter del task
          current_node->data.t.instruction_progress = 0;
          current_node->data.t.program_counter = current_node->data.t.program_counter->next;
          pop(current_node->data.t.instructions);
          // Riporta il processo in READY se c'è una istruzione da eseguire
          if (current_node->data.t.program_counter != NULL) {
            current_node->data.t.state = READY;
            printf("core%d,%d,%d,%s\n", coreNumber, clock, current_node->data.t.id, "READY");
            repeat_analysis = true;
          }
          if (current_node->data.t.state == BLOCKED)
            executing_blocking = false;
          else
            executing_run = false;
        }
      }
      // Rimozione degli EXIT
      Node_t* next_node = current_node->next;
      // Impostazione stato EXIT ai finiti
      if (current_node->data.t.state != NEW && current_node->data.t.program_counter == NULL) {
        current_node->data.t.state = EXIT;
        printf("core%d,%d,%d,%s\n", coreNumber, clock, current_node->data.t.id, "EXIT");
      }
      if (current_node->data.t.state == EXIT) {
        cancelAt(simulation_data, current_node); // Elimino il nodo
      }
      if (repeat_analysis)
        continue;
      current_node = next_node;
    }
    pthread_mutex_unlock(((SchedulerInfo_t*) sched_info)->simulation_data_mutex);
    clock++;
  }
  return NULL;
}

void schedulate(List_t* simulation_data)
{
  // Lancio dei thread
  pthread_t threads[2];
  SchedulerInfo_t thread_data[2];
  pthread_mutex_t simulation_data_mutex;
  pthread_mutex_init(&simulation_data_mutex, NULL);
  for (int i = 0; i < 2; i++) {
    // Creazione parametri per il thread
    thread_data[i].coreNumber = i;
    thread_data[i].scheduler_type = NONPREEMPTIVE;
    thread_data[i].simulation_data = simulation_data;
    thread_data[i].simulation_data_mutex = &simulation_data_mutex;
    // Lancio dei thread
    if (pthread_create(&threads[i], NULL, &schedule, &thread_data[i]) != 0) {
      fprintf(stderr,"%s\n","Non sono riuscito a creare un thread");
      exit(1);
    }
    #ifdef DEBUG
    printf("%s %d\n","Launched thread", i);
    #endif
  }
  // Attesa dei thread
  for (int i = 0; i < 2; i++) {
    pthread_join(threads[i], NULL);
  }
  // Pulizia
  pthread_mutex_destroy(&simulation_data_mutex);
}
