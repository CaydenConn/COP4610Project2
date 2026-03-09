#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <linux/list.h>
#include <linux/mutex.h>

// Elevator states
#define OFFLINE 0
#define IDLE 1
#define LOADING 2
#define UP 3
#define DOWN 4

// Passenger types
#define PART_TIME 0
#define LAWYER 1
#define BOSS 2
#define VISITOR 3

// Constraints
#define MAX_PASSENGERS 5
#define MAX_WEIGHT 70
#define NUM_FLOORS 5

// Weight lookup for passenger types
extern int passenger_weights[4];

// A single passenger
struct passenger {
  int type;        // PART_TIME, LAWYER, BOSS, or VISITOR
  int start_floor; // 1-based floor where the passenger is waiting
  int dest_floor;  // 1-based destination floor
  int weight;      // weight in lbs (copied from passenger_weights)
  struct list_head list;
};

// Elevator state
struct elevator_info {
  int state;                   // OFFLINE, IDLE, LOADING, UP, DOWN
  int current_floor;           // 1-based current floor
  int current_load;            // total weight in elevator
  int num_passengers;          // number of passengers in elevator
  int deactivating;            // Set to if stop_elevator() is called
  struct list_head passengers; // list of passengers struct
  struct mutex lock;
};

// Per-floor waiting queue
struct floor_info {
  int num_waiting;             // number of passengers waiting on this floor
  struct list_head passengers; // list of passengers struct
  struct mutex lock;
};

extern struct elevator_info elevator;
extern struct floor_info floors[NUM_FLOORS];

void init_elevator(void);
void init_floors(void);

int unload_passengers(void);
int load_passengers(void);
void update_direction(void);
bool check_if_should_stop(void);

#endif