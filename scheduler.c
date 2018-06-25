#include "data.c"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

#define CORE_COUNT 2

typedef enum SchedulerType {
  PREEMPTIVE,
  NONPREEMPTIVE
} SchedulerType_t;

typedef struct SchedulerInfo {
  int coreNumber;
  SchedulerType_t scheduler_type;
  List_t* simulation_data;
  FILE* output_file;
  int TIME_SLICE;
} SchedulerInfo_t;

typedef enum SchedulingAlgorithm {
  FCFS,
  SPN
} SchedulingAlgorithm_t;

void statusUpdate(int core, int clock, Node_t* task, ProcessState_t state, FILE* output_file) {
  task->data.t.state = state;
  // output_file = stdout;
  switch (state)
  {
    case NEW:
      fprintf(output_file, "core%d,%d,%d,%s\n", core, clock, task->data.t.id, "new");
      break;
    case READY:
      fprintf(output_file, "core%d,%d,%d,%s\n", core, clock, task->data.t.id, "ready");
      break;
    case RUNNING:
      fprintf(output_file, "core%d,%d,%d,%s\n", core, clock, task->data.t.id, "run");
      break;
    case BLOCKED:
      fprintf(output_file, "core%d,%d,%d,%s\n", core, clock, task->data.t.id, "blocked");
      break;
    case EXIT:
      fprintf(output_file, "core%d,%d,%d,%s\n", core, clock, task->data.t.id, "exit");
      break;
    default:
      break;
  }
}

Node_t* chooseNode(List_t* readyQueue, SchedulingAlgorithm_t algorithm, Node_t* preempted) {
  // FCFS
  if (algorithm == FCFS) {
    Node_t* current_node = readyQueue->head;
    Node_t* choosen_node = readyQueue->head;
    while (current_node != NULL) {
      if (preempted != NULL && preempted == current_node) {
        current_node = current_node->next;
        continue;
      }
      if (current_node->data.t.arrival_time < choosen_node->data.t.arrival_time)
        choosen_node = current_node;
      current_node = current_node->next;
    }
    return choosen_node;
  }
  // SPN
  if (algorithm == SPN) {
    Node_t* current_node = readyQueue->head;
    Node_t* choosen_node = NULL;
    int choosen_node_duration = 0;
    while (current_node != NULL) {
      if (preempted != NULL && preempted == current_node) {
        current_node = current_node->next;
        continue;
      }
      // Calcolo della durata
      int current_node_duration = 0;
      Node_t* current_instruction = current_node->data.t.instructions->head;
      while (current_instruction != NULL) {
        current_node_duration += current_instruction->data.i.length;
        current_instruction = current_instruction->next;
      }
      if (choosen_node == NULL || current_node_duration < choosen_node_duration) {
        choosen_node = current_node;
        choosen_node_duration = current_node_duration;
      }
      current_node = current_node->next;
    }
    return choosen_node;
  }
  // Ignoriamo i task preempted
  preempted = NULL;
  // Impossibile ma altrimenti GCC si altera
  return NULL;
}

void* schedule(void* sched_info) {
  // Prendo i parametri dello scheduler
  int coreNumber = ((SchedulerInfo_t*) sched_info)->coreNumber;
  SchedulerType_t scheduler_type = ((SchedulerInfo_t*) sched_info)->scheduler_type;
  List_t* simulation_data = ((SchedulerInfo_t*) sched_info)->simulation_data;
  FILE* output_file = ((SchedulerInfo_t*) sched_info)->output_file;
  // Variabili di stato
  List_t* readyQueue = newList();
  pthread_cleanup_push(destroyListFromThread, readyQueue); // "Distruttore" lista READY
  List_t* blockedQueue = newList();
  pthread_cleanup_push(destroyListFromThread, blockedQueue); // "Distruttore" lista BLOCKED
  bool terminate = false;
  int clock = 1;
  Node_t* currentlyExecuting = NULL;
  Node_t* preempted = NULL;
  while (!terminate) {
    // Creazione NEW
    pthread_mutex_lock(&simulation_data->mutex); // Inizio della sezione critica
    Node_t* newTask = simulation_data->head;
    while (newTask != NULL) {
      if (clock >= newTask->data.t.arrival_time) {
        statusUpdate(coreNumber, clock, newTask, NEW, output_file);
        newTask->data.t.program_counter = newTask->data.t.instructions->head;
        statusUpdate(coreNumber, clock, newTask, READY, output_file);
        push(readyQueue, popAt(simulation_data, newTask));
      } else {
        break;
      }
      newTask = newTask->next;
    }
    pthread_mutex_unlock(&simulation_data->mutex); // Fine della sezione critica
    // Scelta del task
    if (currentlyExecuting == NULL && readyQueue->head != NULL) {
      Node_t* choosenNode = chooseNode(readyQueue, FCFS, preempted);
      choosenNode->data.t.instruction_progress = clock;
      if (choosenNode->data.t.program_counter->data.i.blocking) {
        // READY -> BLOCKED
        statusUpdate(coreNumber, clock, choosenNode, BLOCKED, output_file);
        push(blockedQueue, popAt(readyQueue, choosenNode));
      } else {
        // READY -> RUNNING
        statusUpdate(coreNumber, clock, choosenNode, RUNNING, output_file);
        currentlyExecuting = popAt(readyQueue, choosenNode);
        if (scheduler_type == PREEMPTIVE) {
          currentlyExecuting->data.t.running_since = clock;
        }
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
          statusUpdate(coreNumber, clock, currentlyExecuting, EXIT, output_file);
          destroyNode(currentlyExecuting);
          currentlyExecuting = NULL;
        } else if (currentlyExecuting->data.t.program_counter->data.i.blocking) {
          // Passo in BLOCKED se incontro una bloccante
          statusUpdate(coreNumber, clock, currentlyExecuting, BLOCKED, output_file);
          push(blockedQueue, currentlyExecuting);
          currentlyExecuting = NULL;
        } else if (scheduler_type == PREEMPTIVE) {
          // RUNNING nel caso PREEMPTIVE
          int TIME_SLICE = ((SchedulerInfo_t*) sched_info)->TIME_SLICE;
          if ((currentlyExecuting->data.t.running_since + TIME_SLICE) <= clock) {
            statusUpdate(coreNumber, clock, currentlyExecuting, READY, output_file);
            push(readyQueue, currentlyExecuting);
            preempted = currentlyExecuting;
            currentlyExecuting = NULL;
            #ifdef DEBUG
            fprintf(output_file, "  PREEMPT\n");
            #endif
          }
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
          // Passo in EXIT se ho finito
          /*
            Q: Supponiamo che un task abbia un'istruzione bloccante come ultima istruzione. Una volta aspettato che si sblocchi, possiamo direttamente mandarlo in "exit"?
            A: No, bisogna loggare e scrivere sul file output che il task, seppur terminato, deve prima passare allo stato "ready" e allo stato "running"
          */
          statusUpdate(coreNumber, clock, blockedTask, READY, output_file);
          statusUpdate(coreNumber, clock, blockedTask, RUNNING, output_file);
          statusUpdate(coreNumber, clock, blockedTask, EXIT, output_file);
          cancelAt(blockedQueue, blockedTask);
        } else {
          // Passo in READY
          statusUpdate(coreNumber, clock, blockedTask, READY, output_file);
          push(readyQueue, popAt(blockedQueue, blockedTask));
        }
        nextBlockedTask = blockedQueue->head;
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

void schedulate(List_t* simulation_data, char* filename, SchedulerType_t scheduler_type)
{
  // Calcolo lunghezza media blocchi istruzioni NB
  int TIME_SLICE;
  if (scheduler_type == PREEMPTIVE) {
    Node_t* current_task = simulation_data->head;
    unsigned int sum = 0.0;
    unsigned int total = 0;
    while (current_task != NULL) {
      Node_t* current_instruction = current_task->data.t.instructions->head;
      bool inBlock = false;
      while (current_instruction != NULL) {
        if (current_instruction->data.i.blocking) {
          if (inBlock) {
            inBlock = false;
            total++;
          }
        } else {
          inBlock = true;
          sum += current_instruction->data.i.length;
        }
        current_instruction = current_instruction->next;
      }
      current_task = current_task->next;
    }
    #ifdef DEBUG
    printf("%i / %i = %f\n", sum, total, sum * 0.8 / total);
    printf("%i / %i = %i\n", sum, total, (int) ceil(sum * 0.8 / total));
    #endif
    TIME_SLICE = (int) ceil(sum * 0.8 / total);
  }
  // Apertura del file di output
  FILE* output_file = fopen(filename, "w");
  if (output_file == NULL) {
    perror("Errore nell'apertura del file");
    exit(1);
  }
  pthread_cleanup_push(closeFile, output_file);
  // Lancio dei thread
  pthread_t threads[CORE_COUNT];
  SchedulerInfo_t thread_data[CORE_COUNT];
  for (int i = 0; i < CORE_COUNT; i++) {
    // Creazione parametri per il thread
    thread_data[i].coreNumber = i;
    thread_data[i].scheduler_type = scheduler_type;
    thread_data[i].simulation_data = simulation_data;
    thread_data[i].output_file = output_file;
    thread_data[i].TIME_SLICE = TIME_SLICE;
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
  // Chiusura dei file
  pthread_cleanup_pop(1);
}
