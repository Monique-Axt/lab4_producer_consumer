# Laboration 4

## Compiling & Running

### make commands

`make` / `make all`

`make clean`

### Command-line args

*Default values (5000 elements, no delay) set if no/invalid arguments are provided.*

**Without switches** (0, 1 or 2 args allowed):

`lab4`

`lab4 5000`

`lab4 3200 1`

**With switches** (2 switches with corresponding args mandatory):

`-it` # items to process, `-d` delay enabled (0/1):

`lab4 -it 3200 -d 0`


## Environment & Tools

A vagrant virtual-box running Ubuntu was used to compile and run the code using a Makefile while remaining on the Windows host machine.

**IDE:**
Intellij Clion 2020.2.4

**OS:**
Windows 10
Ubuntu 18.04

**Compiler:**
g++ clang

**Tools:**
Vagrant 2.2.10
Virtual Box 6.1

## Purpose

A program illustrating the classic producer/consumer design using separate processes that communicate through shared memory and are synchronized using semaphores.

## Procedures

* A `random_device`, `engine` and two `distributions` are set up to handle the generation of random number elements to place into a shared buffer, and to generate random time delays (0-100ms) on each of the two processes.
* All the shared memory management is done using the `SharedMem` class: the shared memory segment is created and attached and the shared buffer and semaphores are placed in the memory segment.
* Class `SharedQueue` implements a circular queue using an array buffer to avoid dynamic memory allocation for the purposes of the shared memory segment.
* Three semaphores are initialised and used: to indicate whether space is available in the shared buffer for production; to indicate whether items are available for consumption; and a binary semaphore to enforce mutual exclusion within critical sections.
* A child process (acting as producer) is then forked from the parent (acting as consumer). Adding or removing items from the shared buffer is considered the critical section.
* If enabled, a random delay occurs once a process has unlocked the mutex (`sem_post(mutex)`). This is implement via a method which can easily be moved to affect delays at different times in each process's code.
* Finally, each process detaches from the shared memory segment and it is removed.
* The above procedures are printed to output with an astrix `*` preceding the message, for illustration purposes.
* Printouts denoting the work of the producer and consumer are printed with PID, element inserted/removed and updated size of the buffer.