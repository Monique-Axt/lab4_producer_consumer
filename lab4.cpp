#include <iostream>
#include "new"
#include <semaphore.h>
#include <cstdio>
#include <unistd.h>
#include <sstream>
#include <random>

#include "Restart.h"
#include "SharedMem.h"
#include "SharedQueue.h"

struct timespec sleeptime{};
const int BUFFSIZE = 10;
const long MILLISECOND = 1000000;
bool delayEnabled = false;
const int DEFAULT_NRITEMS = 5000;
int nrItems = DEFAULT_NRITEMS;
pid_t retPID;

bool cmdLindHandler(int argc, char *argv[]);
void implementDelay(std::uniform_int_distribution<> distr, std::mt19937 gen);
void printYellow(const std::string& msg);
void printDullGreen(const std::string& msg);
void printRed(const std::string& msg);
void setDefaults();

int main(int argc, char* argv[]) {

    if (!cmdLindHandler(argc, argv)){
        std::cout << "Missing or invalid argument/s. Using default values...";
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(1, 99);   // used in producer to generate random numbers
    std::uniform_int_distribution<> distr2(0, 100); // used to create a random time delay

    printRed("Items to process: " + std::to_string(nrItems) + " | Buffer size: " + std::to_string(BUFFSIZE)
             + " | Delay: " + (delayEnabled ? "enabled (0-100ms)" : "disabled"));


    /** Create and attach shared memory segment */
    SharedMem sharedMem(IPC_PRIVATE, (sizeof(SharedQueue<int>)+(3*sizeof(sem_t))));
    printDullGreen("* Shared memory created");
    if (sharedMem.attach() != 0) return 1;
    else printDullGreen("* Shared memory attached: parent process " + std::to_string(getpid()));

    auto* queue = new (sharedMem.getAddr()) SharedQueue<int>;
    sem_t* spaceAvail  = new (sharedMem.getAddr() + sizeof(SharedQueue<int>)) sem_t;
    sem_t* itemAvail   = new (sharedMem.getAddr() + sizeof(SharedQueue<int>) + sizeof(sem_t)) sem_t;
    sem_t *mutex       = new (sharedMem.getAddr() + sizeof(SharedQueue<int>) + sizeof(sem_t) + sizeof(sem_t)) sem_t;

    /** Initialize semaphores */
    if ((sem_init(spaceAvail,1, BUFFSIZE) == -1) ||
        (sem_init(itemAvail, 1, 0) == -1) ||
        (sem_init(mutex, 1, 1) == -1)){
        std::perror("Failed to initialise semaphores");
        return 1;
    } else printDullGreen("* Semaphores initialized");

    /** FORK processes **/
    if ((retPID = fork()) == -1){
        std::perror("Failed to fork");
        return 1;
    }

/** Start: Produce / consume ***************************************/
//CHILD - producer
    if (retPID == 0) {
        if (sharedMem.attach() != 0) return 1;
        else printDullGreen("* Shared memory attached: forked child process " + std::to_string(getpid()));

        int nrProductions = 0;

        while(nrProductions < nrItems) {
            if (sem_wait(spaceAvail) == -1)                             // wait for space to be available
                std::perror("Producer failed to lock semaphore");

            if (sem_wait(mutex) == -1)                                  // START critical section
                std::perror("Producer failed to lock mutex semaphore");

            std::cout << "Prod: " << getpid() << " ";
            if (queue->enqueue(distr(gen))) {                       // try enqueue (produce)
                nrProductions++;
            }
            if (sem_post(mutex) == -1)                                 // END critical section
                std::perror("Producer failed to unlock mutex semaphore");

            implementDelay(distr2, gen);

            if (sem_post(itemAvail) == -1)                             // signal item is available
                std::perror("Producer failed to unlock semaphore");

        }
        printYellow("Produced " + std::to_string(nrProductions) + " items");
    }


// PARENT - consumer
    else {
        int nrConsumptions = 0;

        while(nrConsumptions < nrItems) {
            if (sem_wait(itemAvail) == -1)                              // wait for item to be available
                std::perror("Consumer failed to lock semaphore");

            if (sem_wait(mutex) == -1)                                  // START critical section
                std::perror("Consumer failed to unlock mutex semaphore");

            std::cout << "\tCons: " << getpid() << " ";

            if(queue->dequeue()) {                                     // try dequeue (consume)
                nrConsumptions++;
            }

            if (sem_post(mutex) == -1)                                // END critical section
                std::perror("Consumer failed to unlock mutex semaphore");

            implementDelay(distr2, gen);

            if (sem_post(spaceAvail) == -1)                          // signal space available
                std::perror("Consumer failed to unlock semaphore");


        }
        printYellow("Consumed " + std::to_string(nrConsumptions) + " items");
    }
/** End: Produce / consume ***************************************/

    /** Detach and remove shared memory */
    if (sharedMem.detach() == 0)
        printDullGreen("* Shared memory detached from PID " + std::to_string(getpid()));

    if(r_wait (NULL) == -1)
        return 1;

    if (sharedMem.remove()== 0) //last process to detach calls shmctl()
        printDullGreen("* Shared memory removed");

    printRed("Items processed: " + std::to_string(nrItems) + " | Buffer size: " + std::to_string(BUFFSIZE)
             + " | Delay: " + (delayEnabled ? "enabled (0-100ms)" : "disabled"));

    return 0;
}

void implementDelay(std::uniform_int_distribution<> distr, std::mt19937 gen){
    if (delayEnabled) {
        sleeptime.tv_sec = 0;
        sleeptime.tv_nsec = distr(gen) * MILLISECOND;

        if(nanosleep(&sleeptime, NULL) == -1) {
            perror("nanosleep");
        }
    }
}

void setDefaults() {
    nrItems = DEFAULT_NRITEMS;
    delayEnabled = false;
}

bool cmdLindHandler(int argc, char *argv[]) {
    switch (argc) {
        case 1: return false;                   // 0 args: use default values

        case 2: {                               // 1 arg, no switch: nr of items to process
            std::istringstream istr(argv[1]);
            if (!(istr >> nrItems)) {
                setDefaults();
                return false;
            }
        } break;

        case 3: {                               // 2 arguments, no switch: nr items and delay
            std::istringstream istr(argv[1]);
            std::istringstream istr2(argv[2]);

            if (!(istr >> nrItems) || !(istr2 >> delayEnabled)) {
                setDefaults();
                return false;
            }

        } break;

        default: {                           // 4 args, 2 arguments with switches
            int items = 0;
            bool delay = false;

            for (int i = 1; i < argc; i++) {
                std::string arg = argv[i];

                // get number of items to process
                if (arg == "-it") {
                    std::istringstream istr(argv[i + 1]);

                    if (!(istr >> items)) {
                        return false;
                    }

                    if (items < 1) {
                        return false;
                    }

                    nrItems = items;
                }
                    // get argument for enabling a random delay
                else if (arg == "-d") {
                    std::istringstream istr(argv[i + 1]);
                    if (!(istr >> delay)) {
                        return false;
                    }
                    delayEnabled = delay;
                } else {
                    std::cout << "Unknown switch " << arg << std::endl;
                    std::cout << "Usage: testCmdline [-it #items] [-d 0/1]" << std::endl;
                    return false;
                }
                ++i; // Skip the value go to next switch on command line
            } // for i
        }
    }
    return true;
}


void printRed(const std::string& msg) {

    std::cout << "\x1B[31m" << msg << "\033[0m\t\t" << std::endl;
}

void printDullGreen(const std::string& msg)
{
    std::cout << "\x1B[32m" << msg << "\033[0m"<< std::endl;
}

void printYellow(const std::string& msg)
{
    std::cout << "\x1B[93m" << msg << "\033[0m\n" << std::endl;
}
