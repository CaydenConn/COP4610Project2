#include "scheduler.h"
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define PROC_NAME "elevator"
#define PROC_BUF_SIZE 10000

static struct proc_dir_entry *proc_entry;

// Map passenger type to a single character label
static char type_label(int type) {
  switch (type) {
    case PART_TIME: return 'P';
    case LAWYER:    return 'L';
    case BOSS:      return 'B';
    case VISITOR:   return 'V';
    default:        return '?';
  }
}

// Map elevator state number to a string
static const char *state_string(int state) {
  switch (state) {
    case OFFLINE:  return "OFFLINE";
    case IDLE:     return "IDLE";
    case LOADING:  return "LOADING";
    case UP:       return "UP";
    case DOWN:     return "DOWN";
    default:       return "UNKNOWN";
  }
}

static ssize_t elevator_proc_read(struct file *file, char __user *ubuf,
                                  size_t count, loff_t *ppos) {
  char *buf;
  int len = 0;
  int i, total_waiting = 0;
  struct passenger *p;

  if (*ppos > 0)
    return 0;

  buf = kmalloc(PROC_BUF_SIZE, GFP_KERNEL);
  if (!buf)
    return -ENOMEM;

  // Lock elevator to read its state
  mutex_lock(&elevator.lock);

  len += snprintf(buf + len, PROC_BUF_SIZE - len,
    "Elevator state: %s\n", state_string(elevator.state));
  len += snprintf(buf + len, PROC_BUF_SIZE - len,
    "Current floor: %d\n", elevator.current_floor);
  len += snprintf(buf + len, PROC_BUF_SIZE - len,
    "Current load: %d lbs\n", elevator.current_load);

  // List passengers in elevator
  len += snprintf(buf + len, PROC_BUF_SIZE - len, "Elevator status:");
  list_for_each_entry(p, &elevator.passengers, list) {
    len += snprintf(buf + len, PROC_BUF_SIZE - len,
      " %c%d", type_label(p->type), p->dest_floor);
  }
  len += snprintf(buf + len, PROC_BUF_SIZE - len, "\n");

  // Print each floor (top to bottom, floor 5 down to floor 1)
  len += snprintf(buf + len, PROC_BUF_SIZE - len, "\n");
  for (i = NUM_FLOORS; i >= 1; i--) {
    int idx = i - 1;

    mutex_lock(&floors[idx].lock);

    // Indicator: [*] if elevator is on this floor, [ ] otherwise
    if (elevator.current_floor == i)
      len += snprintf(buf + len, PROC_BUF_SIZE - len, "[*] ");
    else
      len += snprintf(buf + len, PROC_BUF_SIZE - len, "[ ] ");

    len += snprintf(buf + len, PROC_BUF_SIZE - len,
      "Floor %d: %d", i, floors[idx].num_waiting);

    total_waiting += floors[idx].num_waiting;

    // List each waiting passenger on this floor
    list_for_each_entry(p, &floors[idx].passengers, list) {
      len += snprintf(buf + len, PROC_BUF_SIZE - len,
        " %c%d", type_label(p->type), p->dest_floor);
    }

    len += snprintf(buf + len, PROC_BUF_SIZE - len, "\n");

    mutex_unlock(&floors[idx].lock);
  }

  // Summary counts
  len += snprintf(buf + len, PROC_BUF_SIZE - len, "\n");
  len += snprintf(buf + len, PROC_BUF_SIZE - len,
    "Number of passengers: %d\n", elevator.num_passengers);
  len += snprintf(buf + len, PROC_BUF_SIZE - len,
    "Number of passengers waiting: %d\n", total_waiting);
  len += snprintf(buf + len, PROC_BUF_SIZE - len,
    "Number of passengers serviced: %d\n", elevator.passengers_serviced);

  mutex_unlock(&elevator.lock);

  // Copy to user space
  if (len > count)
    len = count;
  if (copy_to_user(ubuf, buf, len)) {
    kfree(buf);
    return -EFAULT;
  }

  *ppos = len;
  kfree(buf);
  return len;
}

static const struct proc_ops elevator_proc_ops = {
  .proc_read = elevator_proc_read,
};

// Called from elevator_init_module to create /proc/elevator
int elevator_proc_init(void) {
  proc_entry = proc_create(PROC_NAME, 0644, NULL, &elevator_proc_ops);
  if (!proc_entry) {
    printk(KERN_ERR "elevator: failed to create /proc/%s\n", PROC_NAME);
    return -ENOMEM;
  }
  return 0;
}

// Called from elevator_exit_module to remove /proc/elevator
void elevator_proc_exit(void) {
  proc_remove(proc_entry);
}
