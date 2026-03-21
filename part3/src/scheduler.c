#include "scheduler.h"
#include <linux/list.h>
#include <linux/slab.h>


int unload_passengers(void) {
  struct passenger *p, *tmp;
  int unloaded = 0;

  // Checks if there are any passengers in the elevator that want to get off at the current floor
  list_for_each_entry_safe(p, tmp, &elevator.passengers, list) {
    if (p->dest_floor == elevator.current_floor) {
      elevator.current_load -= p->weight;
      elevator.num_passengers--;
      elevator.passengers_serviced++;
      list_del(&p->list);
      kfree(p);
      unloaded++;
    }
  }

  return unloaded;
}


int load_passengers(void) {
  struct floor_info *fi;
  struct passenger *p, *tmp;
  int loaded = 0;
  int idx;

  if (elevator.deactivating)
    return 0;

  idx = elevator.current_floor - 1;
  fi = &floors[idx];

  mutex_lock(&fi->lock);

  list_for_each_entry_safe(p, tmp, &fi->passengers, list) {

    if (elevator.num_passengers >= MAX_PASSENGERS) // Checks if elevator is full
      break;
    if (elevator.current_load + p->weight > MAX_WEIGHT) // Checks if elevator is overweight
      break;

    // Removes the passenger from the floor list and adds them to the elevator list
    list_del(&p->list);
    list_add_tail(&p->list, &elevator.passengers);
    elevator.num_passengers++;
    elevator.current_load += p->weight;
    fi->num_waiting--;
    loaded++;
  }

  mutex_unlock(&fi->lock);

  return loaded;
}


static bool has_passengers_above(void) {
  struct passenger *p;
  int i;

  // Checks if there are any passengers in the elevator that want to go to a higher floor
  list_for_each_entry(p, &elevator.passengers, list) {
    if (p->dest_floor > elevator.current_floor)
      return true;
  }

  // Checks if there are any passengers waiting on the floors above the current floor
  for (i = elevator.current_floor; i < NUM_FLOORS; i++) {
    mutex_lock(&floors[i].lock);
    if (floors[i].num_waiting > 0) {
      mutex_unlock(&floors[i].lock);
      return true;
    }
    mutex_unlock(&floors[i].lock);
  }

  return false;
}

static bool has_passengers_below(void) {
  struct passenger *p;
  int i;

  // Checks if there are any passengers in the elevator that want to go to a lower floor
  list_for_each_entry(p, &elevator.passengers, list) {
    if (p->dest_floor < elevator.current_floor)
      return true;
  }

  // Checks if there are any passengers waiting on the floors below the current floor
  for (i = 0; i < elevator.current_floor - 1; i++) {
    mutex_lock(&floors[i].lock);
    if (floors[i].num_waiting > 0) {
      mutex_unlock(&floors[i].lock);
      return true;
    }
    mutex_unlock(&floors[i].lock);
  }

  return false;
}


static bool has_any_waiting(void) {
  int i;

  // Checks if there are any passengers waiting on any floor
  for (i = 0; i < NUM_FLOORS; i++) {
    mutex_lock(&floors[i].lock);
    if (floors[i].num_waiting > 0) {
      mutex_unlock(&floors[i].lock);
      return true;
    }
    mutex_unlock(&floors[i].lock);
  }

  return false;
}


static int find_nearest_waiting_floor(void) {
  int i, best = 0, best_dist = NUM_FLOORS + 1;

  for (i = 0; i < NUM_FLOORS; i++) {
    int floor_num = i + 1;
    int dist;

    mutex_lock(&floors[i].lock);
    if (floors[i].num_waiting > 0) {
      dist = abs(elevator.current_floor - floor_num);
      if (dist < best_dist) {
        best_dist = dist;
        best = floor_num;
      }
    }
    mutex_unlock(&floors[i].lock);
  }

  return best;
}

void update_direction(void) {
  // If deactivating and elevator is empty, go OFFLINE
  if (elevator.deactivating && elevator.num_passengers == 0) {
    elevator.state = OFFLINE;
    elevator.deactivating = 0;
    return;
  }

  // If elevator has passengers or there are waiters, figure out direction
  if (elevator.state == UP) {
    if (has_passengers_above())
      return; // keep going up
    if (has_passengers_below()) {
      elevator.state = DOWN;
      return;
    }
    // No work in either direction
    elevator.state = IDLE;
    return;
  }

  if (elevator.state == DOWN) {
    if (has_passengers_below())
      return; // keep going down
    if (has_passengers_above()) {
      elevator.state = UP;
      return;
    }
    elevator.state = IDLE;
    return;
  }

  // If the state of the elevator is IDLE or LOADING, check for work
  if (elevator.num_passengers > 0) {
    // Passengers have been found so we decide the direction they want to go 
    struct passenger *p;

    list_for_each_entry(p, &elevator.passengers, list) {
      if (p->dest_floor > elevator.current_floor) {
        elevator.state = UP;
        return;
      }
      if (p->dest_floor < elevator.current_floor) {
        elevator.state = DOWN;
        return;
      }
    }
  }

  // No passengers on elevator — check for waiting passengers
  if (has_any_waiting()) {
    int nearest = find_nearest_waiting_floor();

    if (nearest > elevator.current_floor)
      elevator.state = UP;
    else if (nearest < elevator.current_floor)
      elevator.state = DOWN;
    else
      elevator.state = LOADING;
    return;
  }

  // Nothing for the elevator to do 
  elevator.state = IDLE;
}


bool check_if_should_stop(void) {
  struct passenger *p;
  struct floor_info *fi;
  int idx;

  // Checks if anyone needs to get off on the current floor
  list_for_each_entry(p, &elevator.passengers, list) {
    if (p->dest_floor == elevator.current_floor)
      return true;
  }

  // If deactivating, don't stop to pick up new passengers
  if (elevator.deactivating)
    return false;

  // Check if anyone on this floor can board
  idx = elevator.current_floor - 1;
  fi = &floors[idx];

  mutex_lock(&fi->lock);

  list_for_each_entry(p, &fi->passengers, list) {
    if (elevator.num_passengers < MAX_PASSENGERS &&
        elevator.current_load + p->weight <= MAX_WEIGHT) {
      mutex_unlock(&fi->lock);
      return true;
    }
    // Uses FIFO: if the first passenger doesn't fit, stop checking
    break;
  }

  mutex_unlock(&fi->lock);

  return false;
}