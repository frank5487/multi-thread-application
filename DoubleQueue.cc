#include <pthread.h>
#include "./DoubleQueue.h"

// TODO
DoubleQueue::DoubleQueue() : head(nullptr), tail(nullptr), size(0) {
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);
}

DoubleQueue::~DoubleQueue() {
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);

    // Free memory
    QueueNode* cur = head;
    while (cur != nullptr) {
        QueueNode* tmp = cur;
        cur = cur->next;
        delete tmp;
    }

}

void DoubleQueue::add(double val) {
    QueueNode* newNode = new QueueNode;
    newNode->next = nullptr;
    newNode->value = val;

    pthread_mutex_lock(&lock);

    if (tail == nullptr) {
        head = newNode;
        tail = newNode;
    } else {
        tail->next = newNode;
        tail = newNode;
    }

    size++;

    pthread_cond_signal(&cond);

    pthread_mutex_unlock(&lock);
}

bool DoubleQueue::remove(double *ret) {
    pthread_mutex_lock(&lock);
    if (head == nullptr) {
        pthread_mutex_unlock(&lock);
        return false;
    }

    *ret = head->value;
    QueueNode* tmp = head;
    head = head->next;
    delete tmp;

    size--;

    if (head == nullptr) {
        tail = nullptr;
    }
    pthread_mutex_unlock(&lock);

    return true;
}

double DoubleQueue::wait_remove() {
    pthread_mutex_lock(&lock);
    while (head == nullptr) {
        pthread_cond_wait(&cond, &lock);
    }

    double res = head->value;
    QueueNode* tmp = head;
    head = head->next;
    delete tmp;

    size--;

    if (head == nullptr) {
        tail = nullptr;
    }

    pthread_mutex_unlock(&lock);

    return res;
}

int DoubleQueue::length() {
    pthread_mutex_lock(&lock);
    int res = size;
    pthread_mutex_unlock(&lock);

    return res;
}
