#include <iostream>
#include <iomanip>
#include <limits>
#include <ios>
#include <unistd.h>
#include "DoubleQueue.h"

using std::cout;
using std::endl;
using std::fixed;
using std::cin;
using std::setprecision;
using std::numeric_limits;
using std::streamsize;

// Global variables
DoubleQueue queue;
double nums[5];
int size = 0;
bool done = false;

// Thread 1: prompts the user for input and adds it to the queue
void* read_thread(void* arg);

// Thread 2: reads values from the queue and computes statistics
void* print_thread(void* arg);

int main() {
    // create threads
    pthread_t read_tid, print_tid;
    pthread_create(&read_tid, NULL, read_thread, NULL);
    pthread_create(&print_tid, NULL, print_thread, NULL);

    // wait for input thread to finish (i.e. for EOF to be detected)
    pthread_join(read_tid, NULL);

    // signal to stats thread that it can finish
    done = true;

    // wait for stats thread to finish
    pthread_join(print_tid, NULL);

    return EXIT_SUCCESS;
}

void* read_thread(void* arg) {
    while (!done) {
//        cout << "Enter a number: ";
        double value;
        cin >> value;
        while (!cin) {
            if (cin.eof()) {
                done = true;
                // to notify queue with dummy value
                queue.add(0);
                return nullptr;
            }
            cin.clear(); // clear error flags

            // ignore error unput that would cause the error.
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            // try reading value again
            cin >> value;
        }
        queue.add(value);
        // sleep for 1 second after adding a number to the queue
        sleep(1);
    }
    return nullptr;
}


void* print_thread(void* arg) {
    while (true) {
        // wait for a value to be available in the queue
        double val = queue.wait_remove();
        if (done) {
            return nullptr;
        }

//        double val;
//        while (!queue.remove(&val)) {
//            if (done) {
//                return nullptr;
//            }
//        }

        // add value to last_five array
        nums[size % 5] = val;
        // notice that, it might overflow, we can mod the upper bound of Integer
        size++;

        // compute and print statistics
        double sum = 0;
        double max = val;
        double min = val;
        int len = std::min(size, 5);
        for (int i = 0; i < len; i++) {
            sum += nums[(size - i - 1) % 5];
            max = std::max(max, nums[(size - i - 1) % 5]);
            min = std::min(min, nums[(size - i - 1) % 5]);
        }
        double avg = sum / len;

        cout << setprecision(2) << std::fixed;
        cout << "Max: " << max << endl;
        cout << "Min: " << min << endl;
        cout << "Average: " << avg << endl;
        cout << "Last five: ";
        for (int i = len-1; i >= 0; i--) {
            cout << nums[(size - i - 1) % 5] << " ";
        }
        cout << endl;
    }
}