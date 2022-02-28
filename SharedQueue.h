//
// Created by Monique on 2021-01-02.
//

#ifndef LAB4_SHAREDQUEUE_H
#define LAB4_SHAREDQUEUE_H

#include <iostream>

template <typename T>
class SharedQueue {
public:
    SharedQueue() :  head(-1), tail(-1), nrElements(0) { };

    bool isFull() const { return nrElements == BUFFSIZE; };
    bool isEmpty() const { return nrElements == 0; };
    int getSize() const { return nrElements; };

    bool enqueue(T value) {
        if (isFull()) {
            return false;
        }

        else {
            nrElements++;
            if (head == -1) {
                head = 0;                    // first element inserted
            }

            tail = (tail + 1) % BUFFSIZE;    // update tail position
            buffer[tail] = value;            // add element to new tail position

            std::cout << "Inserted [" << buffer[tail] << "]. nBuff=" << nrElements << std::endl;
            return true;
        }
    }

    bool dequeue() {
        if (isEmpty()) {
            return false;
        }

        else {
            auto temp = buffer[head];
            nrElements--;

            if (head == tail) {
                head = -1;
                tail = -1;
            }
            else {
                head = (head + 1) % BUFFSIZE;    // update head position
            }

            std::cout   << "Removed [" << temp << "]. nBuff=" << nrElements << std::endl;
            return true;
        }
    }

    void displayQueue()
    {
        if (head == -1)
        {
            printf("\nQueue is empty");
            return;
        }
        printf(">>>> Elements in queue: ");
        if (tail >= head)
        {
            for (int i = head; i <= tail; i++)
                printf("%d ",buffer[i]);
        }
        else
        {
            for (int i = head; i < nrElements; i++)
                printf("%d ", buffer[i]);

            for (int i = 0; i <= tail; i++)
                printf("%d ", buffer[i]);
        }

        std::cout << std::endl;
    }



private:
   int BUFFSIZE = 10;
    int head;       // dequeue
    int tail;       // enqueue
    T buffer[10];
    int nrElements;
};


#endif //LAB4_SHAREDQUEUE_H
