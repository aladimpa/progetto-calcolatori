#include "data.c"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define CORE_COUNT 1

typedef enum SchedulerType {
  PREEMPTIVE,
  NONPREEMPTIVE
} SchedulerType_t;

typedef struct SchedulerInfo {
  int coreNumber;
  SchedulerType_t scheduler_type;
  List_t* simulation_data;
} SchedulerInfo_t;

void statusUpdate(int core, int clock, Node_t* task, ProcessState_t state) {
  task->data.t.state = state;
  switch (state)
  {
    case NEW:
      printf("core%d,%d,%d,%s\n", core, clock, task->data.t.id, "new");
      break;
    case READY:
      printf("core%d,%d,%d,%s\n", core, clock, task->data.t.id, "ready");
      break;
    case RUNNING:
      printf("core%d,%d,%d,%s\n", core, clock, task->data.t.id, "run");
      break;
    case BLOCKED:
      printf("core%d,%d,%d,%s\n", core, clock, task->data.t.id, "blocked");
      break;
    case EXIT:
      printf("core%d,%d,%d,%s\n", core, clock, task->data.t.id, "exit");
      break;
    default:
      break;
  }
}

void* schedule(void* sched_info) {
  // Prendo i parametri dello scheduler
  int coreNumber = ((SchedulerInfo_t*) sched_info)->coreNumber;
  // SchedulerType_t scheduler_type = ((SchedulerInfo_t*) sched_info)->scheduler_type;
  List_t* simulation_data = ((SchedulerInfo_t*) sched_info)->simulation_data;
  // Variabili di stato
  List_t* readyQueue = newList();
  pthread_cleanup_push(destroyListFromThread, readyQueue); // "Distruttore" lista READY
  List_t* blockedQueue = newList();
  pthread_cleanup_push(destroyListFromThread, blockedQueue); // "Distruttore" lista BLOCKED
  bool terminate = false;
  int clock = 1;
  Node_t* currentlyExecuting = NULL;
  while (!terminate) {
    // Creazione NEW
    pthread_mutex_lock(&simulation_data->mutex); // Inizio della sezione critica
    Node_t* newTask = simulation_data->head;
    while (newTask != NULL) {
      if (clock >= newTask->data.t.arrival_time) {
        statusUpdate(coreNumber, clock, newTask, NEW);
        newTask->data.t.program_counter = newTask->data.t.instructions->head;
        statusUpdate(coreNumber, clock, newTask, READY);
        push(readyQueue, popAt(simulation_data, newTask));
      } else {
        break;
      }
      newTask = newTask->next;
    }
    pthread_mutex_unlock(&simulation_data->mutex); // Fine della sezione critica
    if (currentlyExecuting == NULL && readyQueue->head != NULL) {
      readyQueue->head->data.t.instruction_progress = clock;
      if (readyQueue->head->data.t.program_counter->data.i.blocking) {
        // READY -> BLOCKED
        statusUpdate(coreNumber, clock, readyQueue->head, BLOCKED);
        push(blockedQueue, popAt(readyQueue, readyQueue->head));
      } else {
        // READY -> RUNNING
        statusUpdate(coreNumber, clock, readyQueue->head, RUNNING);
        currentlyExecuting = popAt(readyQueue, readyQueue->head);
      }
    }
    // RUNNING
    if (currentlyExecuting != NULL) {
      if (clock == (currentlyExecuting->data.t.program_counter->data.i.length + currentlyExecuting->data.t.instruction_progress)) {
        // Passo alla prossima istruzione
        currentlyExecuting->data.t.program_counter = currentlyExecuting->data.t.program_counter->next;
        currentlyExecuting->data.t.instruction_progress = clock;
        if (currentlyExecuting->data.t.program_counter == NULL) {
          // Passo in EXIT se ho finito
          statusUpdate(coreNumber, clock, currentlyExecuting, EXIT);
          destroyNode(currentlyExecuting);
          currentlyExecuting = NULL;
        } else if (currentlyExecuting->data.t.program_counter->data.i.blocking) {
          // Passo in BLOCKED se incontro una bloccante
          statusUpdate(coreNumber, clock, currentlyExecuting, BLOCKED);
          push(blockedQueue, currentlyExecuting);
          currentlyExecuting = NULL;
        }
      }
    }
    // BLOCKED
    Node_t* blockedTask = blockedQueue->head;
    while (blockedTask != NULL) {
      Node_t* nextBlockedTask = blockedTask->next;
      if (clock == (blockedTask->data.t.program_counter->data.i.length + blockedTask->data.t.instruction_progress)) {
        blockedTask->data.t.program_counter = blockedTask->data.t.program_counter->next;
        blockedTask->data.t.instruction_progress = clock;
        if (blockedTask->data.t.program_counter == NULL) {
          statusUpdate(coreNumber, clock, blockedTask, EXIT);
          cancelAt(blockedQueue, blockedTask);
        } else {
          statusUpdate(coreNumber, clock, blockedTask, READY);
          push(readyQueue, popAt(blockedQueue, blockedTask));
        }
      }
      blockedTask = nextBlockedTask;
    }
    clock++;
    // Condizione di uscita dal ciclo
    pthread_mutex_lock(&simulation_data->mutex);
    if (simulation_data->head == NULL && readyQueue->head == NULL && blockedQueue->head == NULL && currentlyExecuting == NULL)
      terminate = true;
    pthread_mutex_unlock(&simulation_data->mutex);
    /*
    if (readyQueue->head != NULL) {
      printf("READY\n"); printList(readyQueue);
    }
    if (blockedQueue->head != NULL) {
      printf("BLOCK\n"); printList(blockedQueue);
    }
    */
  }
  pthread_cleanup_pop(1); // Esegue il distruttore lista BLOCKED
  pthread_cleanup_pop(1); // Esegue il distruttore lista READY
  pthread_exit(NULL);
}

void schedulate(List_t* simulation_data)
{
  // Lancio dei thread
  pthread_t threads[CORE_COUNT];
  SchedulerInfo_t thread_data[CORE_COUNT];
  for (int i = 0; i < CORE_COUNT; i++) {
    // Creazione parametri per il thread
    thread_data[i].coreNumber = i;
    thread_data[i].scheduler_type = NONPREEMPTIVE;
    thread_data[i].simulation_data = simulation_data;
    // Lancio dei thread
    if (pthread_create(&threads[i], NULL, &schedule, &thread_data[i]) != 0) {
      fprintf(stderr,"%s\n","Non sono riuscito a creare un thread");
      exit(1);
    }
  }
  // Attesa dei thread
  for (int i = 0; i < CORE_COUNT; i++) {
    pthread_join(threads[i], NULL);
  }
}
