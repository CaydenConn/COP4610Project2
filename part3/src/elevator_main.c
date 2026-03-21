#include "scheduler.h"
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/types.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("COP4610");

// functio pointers defined in syscalls.c
extern int (*STUB_start_elevator)(void);
extern int (*STUB_issue_request)(int, int, int);
extern int (*STUB_stop_elevator)(void);

// Thread pointer
static struct task_struct *elevator_thread;

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
  elevator.passengers_serviced = 0;
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

static int elevator_thread_run_fn(void *data){
  while(!kthread_should_stop()) {
    bool did_load = false;
    bool did_move = false;
  
    mutex_lock(&elevator.lock);

    // if elevator is offline, unlock and exit thread
    if (elevator.state == OFFLINE) {
      mutex_unlock(&elevator.lock);
      break;
    }

    if(check_if_should_stop()) {
      elevator.state = LOADING;
      // unload passengers whose destination is the current floor
      unload_passengers();

      // load passengers waiting on the current floor
      load_passengers();
      did_load = true;
    }

    update_direction();

    // move elevator
    if(elevator.state == UP && elevator.current_floor < NUM_FLOORS){
      elevator.current_floor++;
      did_move = true;
    } else if(elevator.state == DOWN && elevator.current_floor > 1){
      elevator.current_floor--;
      did_move = true;
    }

    mutex_unlock(&elevator.lock);
    
    // elevator waits for 1 second if passengers are loading/unloading
    if(did_load){
      msleep_interruptible(1000);
    }
    // elevator waits for 2 seconds if it moved floors
    if(did_move){
      msleep_interruptible(2000);
    }
    if (!did_load && !did_move){
      msleep_interruptible(100);
    }
  }

  return 0;
}

// module initialization
static int __init elevator_init_module(void){
  int ret;

  init_elevator();
  init_floors();

  ret = elevator_proc_init();
  if (ret)
    return ret;

  // syscalls are set to the syscalls implemented in this file
  STUB_start_elevator = start_elevator;
  STUB_issue_request = issue_request;
  STUB_stop_elevator = stop_elevator;

  return 0;
}

// module cleanup
static void __exit elevator_exit_module(void){
  struct list_head *curr, *next;
  struct passenger *p;
  int i;

  // stops elevator thread if it is still running
  if(elevator_thread){
    kthread_stop(elevator_thread);
  }
  
  STUB_start_elevator = NULL;
  STUB_issue_request = NULL;
  STUB_stop_elevator = NULL;

  // remove /proc/elevator entry
  elevator_proc_exit();

  // free elevator passengers
  mutex_lock(&elevator.lock);

  // sets curr to be the first element of the list
  curr = elevator.passengers.next;

  // checks to see if curr != head of the list, if it does it means the entire list has been freed
  while(curr != &elevator.passengers){
    next = curr->next;
    p = list_entry(curr, struct passenger, list);
    list_del(curr);
    kfree(p);
    curr = next;
  }

  mutex_unlock(&elevator.lock);

  // free floor passengers
  for(i = 0; i < NUM_FLOORS; i++){
    mutex_lock(&floors[i].lock);
    
    // sets curr to be the first element of the list
    curr = floors[i].passengers.next;
    
    // checks to see if curr != head of the list, if it does it means the entire list has been freed
    while(curr != &floors[i].passengers){
      next = curr->next;
      p = list_entry(curr, struct passenger, list);
      list_del(curr);
      kfree(p);
      curr = next;
    }

    mutex_unlock(&floors[i].lock);
  }
}

// syscall: start elevator
int start_elevator(void){
  mutex_lock(&elevator.lock);
  
  // if elevator is already running unlock and return 1
  if(elevator.state != OFFLINE){
    mutex_unlock(&elevator.lock);
    return 1;
  }

  // initialize default elevator state
  elevator.state = IDLE;
  elevator.current_floor = 1;
  elevator.current_load = 0;
  elevator.num_passengers = 0;
  elevator.passengers_serviced = 0;
  elevator.deactivating = 0;

  mutex_unlock(&elevator.lock);

  // start kenrel thread
  elevator_thread = kthread_run(elevator_thread_run_fn, NULL, "elevator");
  
  if (IS_ERR(elevator_thread))
    return -ENOMEM;
  
  return 0;
}

// syscall: issue request
int issue_request(int start, int dest, int type){
  struct passenger *p;
  int idx;
  
  // ensures that the passed arguments are valid
  if(start < 1 || start > NUM_FLOORS ||
    dest < 1 || dest > NUM_FLOORS || 
    start == dest ||
    type < 0 || type > 3
  ){
    return 1;
  }

  // allocate passenger
  p = kmalloc(sizeof(struct passenger), GFP_KERNEL);
  if (!p){
    return -ENOMEM;
  }

  // initialize linked list
  INIT_LIST_HEAD(&p->list);

  // fill in passenger data from arguments
  p->type = type;
  p->start_floor = start;
  p->dest_floor = dest;
  p->weight = passenger_weights[type];

  idx = start - 1;

  // add passenger to floor queue
  mutex_lock(&floors[idx].lock);
  list_add_tail(&p->list, &floors[idx].passengers);
  floors[idx].num_waiting++;
  mutex_unlock(&floors[idx].lock);

  return 0;
}

// syscall: stop elevator
int stop_elevator(void){
  mutex_lock(&elevator.lock);
  // if already stopping, unlock and return 1
  if(elevator.deactivating){
    mutex_unlock(&elevator.lock);
    return 1;
  }

  // mark elevator for shutdown
  elevator.deactivating = 1;
  mutex_unlock(&elevator.lock);
  return 0;
}

// register module entry/exit points
module_init(elevator_init_module);
module_exit(elevator_exit_module);