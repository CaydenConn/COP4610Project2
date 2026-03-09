# COP4610 Project 2: Elevator Kernel Module

## Project Description
This project focuses on kernel-level programming, concurrency, and system calls in a Linux environment. It is divided into three main components: 
1. **System-Call Tracing:** Verifying and tracing system calls using the `strace` tool.
2. **Timer Kernel Module:** Creating a loadable kernel module (LKM) that creates a `/proc/timer` entry to track the current and elapsed time using kernel time functions.
3. **Elevator Module:** Implementing an elevator scheduling algorithm within the kernel. This module utilizes custom system calls, kernel threads (`kthreads`), mutexes for safe concurrent data access, and linked lists to manage passenger queues and weights.

## Student Information
* **Name:** Cayden Conn
* **FSUID:** cbc22b

* **Name:** Chase Blancher
* **FSUID:** cgb22

* **Name:** Giovanni Giannone
* **FSUID:** gg22e

## Getting Started
1. Ensure you are running the custom compiled `linux-kernel-6.16.x` with the custom system calls installed (Syscalls 548, 549, and 550).
2. Execute `make` in the root directory to compile the trace programs and both kernel modules.
3. **Part 1 (Tracing):** Run `strace -o part1.trace ./part1/part1`
4. **Part 2 (Timer):** Insert the module using `sudo insmod part 2/my_timer.ko`. View the time by running `cat /proc/timer`. Remove with `sudo rmmod my_timer`.
5. **Part 3 (Elevator):** Insert the module using `sudo insmod part 3/elevator.ko`. Use the provided `producer` and `consumer` executables to issue requests and start/stop the elevator. Monitor the state using `watch -n 1 cat /proc/elevator`.

## Supported Features / Components

1. **System-Call Tracing (Part 1)**
   * Empty C program vs. a C program with exactly 5 added system calls.
   * Outputs verified using `strace`.

2. **Timer Kernel Module (Part 2)**
   * Uses `ktime_get_real_ts64()` to fetch kernel time.
   * Procfs interface (`/proc/timer`) that displays both the current Unix Epoch time and the elapsed time since the last read.

3. **Elevator Kernel Module (Part 3)**
   * **System Calls:** Integrated `start_elevator()`, `issue_request()`, and `stop_elevator()`.
   * **Concurrency:** Uses a `kthread` to run the elevator background loop and `mutex` locks to prevent race conditions between the producer (system calls) and consumer (elevator thread).
   * **Dynamic Memory & Queues:** Uses `kmalloc` and `linux/list.h` to dynamically allocate passengers and manage First-In-First-Out (FIFO) floor queues.
   * **Scheduling & Constraints:** SCAN-based scheduling algorithm that strictly enforces a 5-passenger and 70 lbs maximum weight limit. Passenger types (Part-time, Lawyer, Boss, Visitor) carry specific weights.
   * **Procfs Interface:** `/proc/elevator` accurately displays elevator state (IDLE, UP, DOWN, LOADING, OFFLINE), current floor, current weight, passenger list, and floor wait queues.

## File Structure & Listing

* **`part 1/`**
  * `empty.c`: Empty C program baseline.
  * `empty.trace`: Trace output of the empty program.
  * `part1.c`: C program executing exactly 5 system calls.
  * `part1.trace`: Trace output verifying the 5 added calls.
  * `Makefile`: Compiles the Part 1 executables.
* **`part 2/`**
  * `src/my_timer.c`: Source code for the timer kernel module.
  * `Makefile`: Compiles the timer kernel module into `my_timer.ko`.
* **`part 3/`**
  * `src/`: Directory containing the elevator module source code (`elevator.c`, `scheduler.c`, etc.).
  * `syscalls.c`: A copy of the system calls implemented in the kernel tree for grading.
  * `Makefile`: Compiles the elevator kernel module into `elevator.ko`.
* **`Makefile`**: Top-level script to build all parts of the project.
* **`README.md`**: Project documentation (this file).

## Division of Labor

**Total Contributions:**

**Cayden Conn** was responsible for tracing system calls and managing the core data structures and thread safety of the elevator module.
* Part 1: System Call Tracing
* Part 3d: Linked List Implementation
* Part 3e: Mutexes and Concurrency Safety

| Date       | Work Completed / Notes                                                      |
|------------|-----------------------------------------------------------------------------|
| 2026-03-XX |                                                                             |
| 2026-03-XX |                                                                             |

**Chase Blancher** was responsible for the time-tracking kernel module and the logical decision-making engine of the elevator.
* Part 2: Timer Kernel Module
* Part 3f: Elevator Scheduling Algorithm

| Date       | Work Completed / Notes                                                      |
|------------|-----------------------------------------------------------------------------|
| 2026-03-XX |                                                                             |
| 2026-03-XX |                                                                             |

**Giovanni Giannone** was responsible for modifying the core Linux kernel, managing the build process, and establishing the background worker thread.
* Part 3a: Adding System Calls
* Part 3b: Kernel Compilation
* Part 3c: Kthreads (Kernel Threads)

| Date       | Work Completed / Notes                                                      |
|------------|-----------------------------------------------------------------------------|
| 2026-03-XX |                                                                             |
| 2026-03-XX |                                                                             |

## Meetings

| Date       | Attendees                                      | Topics Discussed                   | Outcomes / Decisions                     |
|------------|------------------------------------------------|------------------------------------|------------------------------------------|
| 2026-03-XX | Giovanni Giannone, Chase Blancher, Cayden Conn | Initial planning & Kernel compile  | Divided labor and started compilation.   |
|            |                                                |                                    |                                          |

## Known Issues
* None at this time.
