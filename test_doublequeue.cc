/*
 * Copyright ©2023 Travis McGaha.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Pennsylvania
 * CIT 5950 for use solely during Spring Semester 2023 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include <unistd.h>
#include <pthread.h>
#include <iostream>

#include "gtest/gtest.h"
#include "./test_suite.h"

#include "./DoubleQueue.h"

using std::cerr;
using std::endl;

namespace hw2 {

// lock used for both num_write num_read and cerr
static pthread_mutex_t rw_lock;
static int num_read = 0;
static int num_write = 0;

static void* read_doubles(void*);

class Test_DoubleQueue : public ::testing::Test {
 protected:
  // Code here will be called before all tests execute (ie, before
  // any TEST_F).
  virtual void SetUp() { }

  // Code here will be called after each test executes (ie, after
  // each TEST_F)
  virtual void TearDown() {
    // Verify that none of the tests modifies any of the
    // testing constants.
    ASSERT_EQ(1.0, kOne);
    ASSERT_EQ(0.0, kZero);
    ASSERT_EQ(-1.25, kNegative);
    ASSERT_EQ(3.14, kPi);
    ASSERT_EQ(152.0, kRs);
  }

 public:
  // These values are used as constants for the DoubleQueue tests.
  // They cannot be const, as stored value pointers are non-const.
  static double kOne, kZero, kNegative, kPi, kRs;

};  // class Test_DoubleQueue

// statics
double Test_DoubleQueue::kOne = 1.0;
double Test_DoubleQueue::kZero = 0.0;
double Test_DoubleQueue::kNegative = -1.25;
double Test_DoubleQueue::kPi = 3.14;
double Test_DoubleQueue::kRs = 152.0;

TEST_F(Test_DoubleQueue, add_remove) {
  HW2Environment::OpenTestCase();
  DoubleQueue* q = new DoubleQueue();
  double value;

  // try removing before anything has happened
  ASSERT_FALSE(q->remove(&value));

  q->add(kOne);
  ASSERT_EQ(1, q->length());

  ASSERT_TRUE(q->remove(&value));
  ASSERT_EQ(kOne, value);

  HW2Environment::AddPoints(5);

  ASSERT_EQ(0, q->length());
  // try removing from an empty queue
  ASSERT_FALSE(q->remove(&value));

  q->add(kZero);
  q->add(kRs);
  q->add(kNegative);
  q->add(kPi);
  ASSERT_EQ(4, q->length());

  ASSERT_TRUE(q->remove(&value));
  ASSERT_EQ(kZero, value);
  ASSERT_EQ(3, q->length());

  ASSERT_TRUE(q->remove(&value));
  ASSERT_EQ(kRs, value);
  ASSERT_EQ(2, q->length());

  // Delete q to make sure destructor works, then award points
  // if it doesn't crash
  delete q;

  // re-construct and test destructing a queue with 1 elment
  q = new DoubleQueue();
  q->add(kPi);
  delete q;

  // re-construct and test destructing an empty queue
  q = new DoubleQueue();
  delete q;

  HW2Environment::AddPoints(5);
}

TEST_F(Test_DoubleQueue, wait_remove) {
  HW2Environment::OpenTestCase();
  DoubleQueue* q = new DoubleQueue();

  pthread_t reader;
  pthread_create(&reader, nullptr, read_doubles, q);
  pthread_mutex_init(&rw_lock, nullptr);

  // first test that the reader thread sleeps/waits
  // when it tries to read and there is nothing on the queue

  // sleep for a bit to give a chance for reader to start 
  // and to try and wait_remove from q
  sleep(1);

  // ensure that the reader thread
  // has not read anything
  pthread_mutex_lock(&rw_lock);
  ASSERT_EQ(num_read, 0);
  pthread_mutex_unlock(&rw_lock);

  // add and sleep so that reader
  // has a chance to read
  q->add(kPi);
  sleep(1);

  pthread_mutex_lock(&rw_lock);
  num_write = 1;
  if (num_read != 1) {
    cerr << "wait_remove seemingly doesn't notice the addition of";
    cerr << "a double to the queue. Possible deadlock or not implemented";
    cerr << endl;
  }
  ASSERT_EQ(1, num_read);
  pthread_mutex_unlock(&rw_lock);
  HW2Environment::AddPoints(10);

  // next, test the case where there are already
  // values in the queue when the reader calls wait_remove.
  pthread_mutex_lock(&rw_lock);
  q->add(kOne);
  q->add(kNegative);
  q->add(kZero);
  q->add(kRs);
  num_write += 4;
  pthread_mutex_unlock(&rw_lock);

  // sleep for a bit to give a chance for reader to start 
  // and to try and wait_remove from q
  sleep(3);

  int secs_waited;
  for (secs_waited = 3; secs_waited < 30; secs_waited += 3) {
    pthread_mutex_lock(&rw_lock);
    if (num_read == 5) {
      pthread_mutex_unlock(&rw_lock);
      break;
    }
    pthread_mutex_unlock(&rw_lock);
    sleep(3);
  }

  // make sure that we didn't time out
  // waiting for the reader thread to
  // read 4 more times.
  ASSERT_TRUE(secs_waited < 30);
  
  bool* reader_result;
  pthread_join(reader, reinterpret_cast<void**>(&reader_result));

  // make sure the reader had no errors
  ASSERT_TRUE(*reader_result);

  delete reader_result;
  delete q;

  HW2Environment::AddPoints(10);
}

void* read_doubles(void* arg) {
  DoubleQueue* q = static_cast<DoubleQueue*>(arg);
  double value;

  // will be returned to make sure everything worked
  // form the perspective of the reader
  bool* result = new bool(true);

  value = q->wait_remove();
  pthread_mutex_lock(&rw_lock);
  if (value != Test_DoubleQueue::kPi) {
    *result = false;
    cerr << "Incorrect value from returned from call to wait_remove()";
    cerr << "from the reader thread" << endl;
    cerr << "\tExpected: " << Test_DoubleQueue::kPi << endl;
    cerr << "\tActual: " << value << endl;
  }
  num_read += 1;
  pthread_mutex_unlock(&rw_lock);

  while (true) {
    pthread_mutex_lock(&rw_lock);
    if (num_write == 5) {
      pthread_mutex_unlock(&rw_lock);
      break;
    }
    pthread_mutex_unlock(&rw_lock);
    sleep(1);
  }

  value = q->wait_remove();
  pthread_mutex_lock(&rw_lock);
  if (value != Test_DoubleQueue::kOne) {
    *result = false;
    cerr << "Incorrect value from returned from call to wait_remove()";
    cerr << "from the reader thread" << endl;
    cerr << "\tExpected: " << Test_DoubleQueue::kOne << endl;
    cerr << "\tActual: " << value << endl;
  }
  num_read += 1;
  pthread_mutex_unlock(&rw_lock);

  value = q->wait_remove();
  pthread_mutex_lock(&rw_lock);
  if (value != Test_DoubleQueue::kNegative) {
    *result = false;
    cerr << "Incorrect value from returned from call to wait_remove()";
    cerr << "from the reader thread" << endl;
    cerr << "\tExpected: " << Test_DoubleQueue::kNegative << endl;
    cerr << "\tActual: " << value << endl;
  }
  num_read += 1;
  pthread_mutex_unlock(&rw_lock);

  value = q->wait_remove();
  pthread_mutex_lock(&rw_lock);
  if (value != Test_DoubleQueue::kZero) {
    *result = false;
    cerr << "Incorrect value from returned from call to wait_remove()";
    cerr << "from the reader thread" << endl;
    cerr << "\tExpected: " << Test_DoubleQueue::kZero << endl;
    cerr << "\tActual: " << value << endl;
  }
  num_read += 1;
  pthread_mutex_unlock(&rw_lock);

  value = q->wait_remove();
  pthread_mutex_lock(&rw_lock);
  if (value != Test_DoubleQueue::kRs) {
    *result = false;
    cerr << "Incorrect value from returned from call to wait_remove()";
    cerr << "from the reader thread" << endl;
    cerr << "\tExpected: " << Test_DoubleQueue::kRs << endl;
    cerr << "\tActual: " << value << endl;
  }
  num_read += 1;
  pthread_mutex_unlock(&rw_lock);
  
  return result;
}

}  // namespace hw2
