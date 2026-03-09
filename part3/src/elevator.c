#include "scheduler.h"
#include <linux/module.h>
#include <linux/slab.h>

// Global instances of shared data
struct elevator_info elevator;
struct floor_info floors[NUM_FLOORS];

// Weight lookup for passenger types
int passenger_weights[4] = {10, 15, 20, 5};

// Initialize the elevator to its default state
void init_elevator(void) {
  elevator.state = OFFLINE;
  elevator.current_floor = 1;
  elevator.current_load = 0;
  elevator.num_passengers = 0;
  elevator.deactivating = 0;
  INIT_LIST_HEAD(&elevator.passengers);
  mutex_init(&elevator.lock);
}

// Initialize all floor waiting queues
void init_floors(void) {
  int i;

  for (i = 0; i < NUM_FLOORS; i++) {
    floors[i].num_waiting = 0;
    INIT_LIST_HEAD(&floors[i].passengers);
    mutex_init(&floors[i].lock);
  }
}
