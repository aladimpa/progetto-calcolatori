#include "data.c"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
  List_t* simulation_data = newList();
  for(int i = 0; i < 10; i++) {
    Node_t* new_node = newTaskNode(i+5, i+5);
    for(int j = 0; j < 10; j++) {
      push(new_node->data.t.instructions, newInstructionNode(false, j+1));
    }
    push(simulation_data, new_node);
  }
  int clock = 0;
  while (true) {
    printf("Clock: %d\n", clock);
    if (simulation_data->head == NULL)
      break;
    Node_t* current_node = simulation_data->head;
    while(current_node != NULL) {
      // Spostamento dei NEW in READY
      if (clock >= current_node->data.t.arrival_time && current_node->data.t.state == NEW) {
        printf("core%d,%d,%d,%s\n", 0, clock, current_node->data.t.id, "NEW");
        current_node->data.t.program_counter = current_node->data.t.instructions->head;
        current_node->data.t.state = READY;
        printf("core%d,%d,%d,%s\n", 0, clock, current_node->data.t.id, "READY");
      }
      // Impostazione stato EXIT ai finiti
      if (current_node->data.t.state != NEW && current_node->data.t.program_counter == NULL) {
        current_node->data.t.state = EXIT;
        printf("core%d,%d,%d,%s\n", 0, clock, current_node->data.t.id, "EXIT");
        // ELIMINARE IL NODO
      }
      // Esecuzione di uno in RUN
      if (current_node->data.t.state == READY) {
        // Ho a disposizione una istruzione
        if (current_node->data.t.program_counter->data.i.blocking) {
          current_node->data.t.state = BLOCKED;
          printf("core%d,%d,%d,%s\n", 0, clock, current_node->data.t.id, "BLOCKED");
        } else {
          current_node->data.t.state = RUNNING;
          printf("core%d,%d,%d,%s\n", 0, clock, current_node->data.t.id, "RUN");
          current_node->data.t.instruction_progress++;
        }
      }
      // Rimozione degli EXIT
      current_node = current_node->next;
    }
    clock++;
    if (clock > 100)
      break;
  }
  destroyList(simulation_data);
  return 0;
}
