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
3. **Part 1 (Tracing):** The Makefile compiles (and cleans) the executables and the .trace files
4. **Part 2 (Timer):** Insert the module using `sudo insmod part2/my_timer.ko`. View the time by running `cat /proc/timer`. Remove with `sudo rmmod my_timer`.
5. **Part 3 (Elevator):** Insert the module using `sudo insmod part3/src/elevator.ko`. Use the provided `producer` and `consumer` executables in the `tests/` directory to issue requests and start/stop the elevator. Monitor the state using `watch -n 1 cat /proc/elevator`.

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
   * **Scheduling & Constraints:** Scheduling algorithm that enforces a 5-passenger and 70 lbs maximum weight limit. Passenger types (Part-time, Lawyer, Boss, Visitor) carry specific weights.
   * **Procfs Interface:** `/proc/elevator` accurately displays elevator state (IDLE, UP, DOWN, LOADING, OFFLINE), current floor, current weight, passenger list, and floor wait queues.

## File Structure & Listing

* **`part1/`**
  * `empty.c`: Empty C program baseline.
  * `empty.trace`: Trace output of the empty program.
  * `part1.c`: C program executing exactly 5 system calls.
  * `part1.trace`: Trace output verifying the 5 added calls.
  * `Makefile`: Compiles the Part 1 executables.
* **`part2/`**
  * `Makefile`: Compiles the timer kernel module into `my_timer.ko`.
* **`part2/src/`**
  * `my_timer.c`: Source code for the timer kernel module.
* **`part3/`**
  * `Makefile`: Compiles the elevator kernel module into `elevator.ko`.
* **`part3/src/`**
  * `elevator_main.c`: Core module initialization, exit, and system call implementations.
  * `elevator_proc.c`: Logic for generating the `/proc/elevator` output.
  * `scheduler.c`: Elevator scheduling algorithm and linked list management.
  * `scheduler.h`: Header file containing structs, macros, and function prototypes.
  * `syscalls/`: Directory containing the modified kernel system call files (`syscalls.c`, `syscall_64.tbl`, etc.).
  * `Makefile`: Compiles the elevator kernel module into `elevator.ko`.
* **`part3/tests/`**
  * Contains user-space testing applications (`producer.c`, `consumer.c`, and `syscheck.c`).
* **`Makefile`**: Top-level script to build all parts of the project.

## Division of Labor

**Cayden Conn** 
* Part 1: System Call Tracing
* Part 3a: Adding System Calls
* Part 3d: Linked List Implementation
* Part 3e: Mutexes and Concurrency Safety

| Date       | Work Completed / Notes                                                      |
|------------|-----------------------------------------------------------------------------|
| 2026-03-12 | Completed Part 1 tracing and generated .trace files.                        |
| 2026-03-16 | Setup `list_head` structures and memory allocation for passengers.          |
| 2026-03-18 | Implemented system calls and integrated mutex locks for protection.         |

**Chase Blancher** 
* Part 2: Timer Kernel Module
* Part 3f: Elevator Scheduling Algorithm

| Date       | Work Completed / Notes                                                      |
|------------|-----------------------------------------------------------------------------|
| 2026-03-14 | Completed Part 2, implementing `/proc/timer` and `ktime_get_real_ts64()`.   |
| 2026-03-19 | Drafted the core scanning and passenger loading algorithm.                  |
| 2026-03-20 | Finalized `update_direction` and `check_if_should_stop` scheduling logic.   |

**Giovanni Giannone** 
* Part 3a: Adding System Calls
* Part 3b: Kernel Compilation
* Part 3c: Kthreads (Kernel Threads)

| Date       | Work Completed / Notes                                                      |
|------------|-----------------------------------------------------------------------------|
| 2026-03-10 | Successfully compiled the `linux-kernel-6.16.x` kernel.                     |
| 2026-03-13 | Added system calls 548, 549, 550 to the kernel table and header files.      |
| 2026-03-19 | Implemented the background `kthread` loop for the elevator module.          |

## Meetings

| Date       | Attendees                                      | Topics Discussed                   | Outcomes / Decisions                     |
|------------|------------------------------------------------|------------------------------------|------------------------------------------|
| 2026-03-09 | Giovanni Giannone, Chase Blancher, Cayden Conn | Initial planning & Kernel compile  | Divided labor and started compilation.   |
| 2026-03-15 | Giovanni Giannone, Chase Blancher, Cayden Conn | System Call integration & Structs  | Connected custom syscalls to the module. |
| 2026-03-20 | Giovanni Giannone, Chase Blancher, Cayden Conn | Module debugging & Procfs styling  | Resolved concurrency bugs and finalized. |

## Known Issues
* None at this time.
