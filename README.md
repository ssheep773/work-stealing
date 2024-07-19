# Work-Stealing 
### Project Description
This project implements a work-stealing scheduler in C to calculate Fibonacci numbers using multiple threads. The scheduler dynamically balances the workload among threads, ensuring efficient use of computational resources.

HackMD: https://hackmd.io/@sysprog/SJoqB8Y80

### Features
Work Stealing: Threads can steal tasks from each other's queues to balance the workload dynamically.
Multithreading: Utilizes the POSIX threads library (pthread) to run tasks concurrently.
Atomic Operations: Ensures thread safety with atomic operations provided by the C11 standard.
Dynamic Task Generation: Tasks for calculating Fibonacci numbers are generated dynamically and pushed into the work queues.
### Getting Started
#### Prerequisites
GCC Compiler
POSIX Threads Library (pthread)
On Ubuntu, you can install these using:
```shell
sudo apt-get update
sudo apt-get install gcc make
```
#### Building the Project
Clone the repository
```
git clone https://github.com/yourusername/work-stealing-scheduler.git
cd work-stealing-scheduler
```
Compile the project using make:

```
make
```
#### Running the Program
Execute the program with a Fibonacci number as an argument:

```
./main <fib_num>
```
Replace <fib_num> with the Fibonacci number you want to calculate.

Example:
```
$ ./main 15

...
worker 1 finished
worker 4 finished
worker 2 finished
worker 0 finished
workqueue 0 size 16
workqueue 1 size 64
workqueue 2 size 32
workqueue 3 size 64
workqueue 4 size 64
workqueue 5 size 32
workqueue 6 size 16
workqueue 7 size 16
Finish task Fib(15) = 610
```

## Code Structure
* workstealing-main.c: Contains the main function and the implementation of the work-stealing scheduler.
* workstealing.c: Contains the helper functions and data structures for the work-stealing mechanism.
## Important Functions
* void do_work(int id, work_t *work): Executes a task and continues to execute subsequent tasks.
* void *thread(void *payload): The worker thread function that performs work-stealing.
* work_t *join_work(work_t *work): Checks if a task's dependencies are ready to be executed.
* work_t *fib(work_t *w): Generates tasks for calculating Fibonacci numbers.
* work_t *sum(work_t *w): Sums the results of Fibonacci calculations.
* work_t *done_task(work_t *w): Finalizes the computation and stores the result.

## Clean Up
To clean the build artifacts, run:
```
make clean
```
## References
This project is based on the implementation and concepts from:
* [Work-Stealing Example by jserv](https://gist.github.com/jserv/111304e7c5061b05d4d29a47571f7a98)
* The paper "Cilk: An Efficient Multithreaded Runtime System"
