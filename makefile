# Copyright ©2023 Travis McGaha.  All rights reserved.  Permission is
# hereby granted to students registered for University of Pennsylvania
# CIT 5950 for use solely during Spring Semester 2023 for purposes of
# the course.  No other use, copying, distribution, or modification
# is permitted without prior written consent. Copyrights for
# third-party components of this work must be honored.  Instructors
# interested in reusing these course materials should contact the
# author.

# define the commands we will use for compilation and library building
CC = gcc
CXX = g++

# define useful flags to cc/ld/etc.
CFLAGS += -g -Wall -Wpedantic -I. -I.. -std=c11 -O0
CXXFLAGS += -g -Wall -Wpedantic -I. -I.. -std=c++11 -O0
CPPUNITFLAGS = -L../gtest -lgtest

# define common dependencies
OBJS_P1 = cqdbmp.o qdbmp.o
HEADERS_P1 = cqbmp.h qdbmp.h
OBJS_P2 = DoubleQueue.o numbers.o
HEADERS_P2 = DoubleQueue.h
TESTOBJS = test_doublequeue.o test_suite.o

EXECS = test_suite numbers sequential_numbers negative blur_sequential blur_parallel compare_bmp

# compile everything; this is the default rule that fires if a user
# just types "make" in the same directory as this Makefile
all: $(EXECS)

# part 1
negative: $(OBJS_P1) negative.cc
	$(CXX) $(CXXFLAGS) -o negative negative.cc $(OBJS_P1)

blur_sequential: $(OBJS_P1) blur_sequential.cc
	$(CXX) $(CXXFLAGS) -o blur_sequential blur_sequential.cc $(OBJS_P1)

blur_parallel: $(OBJS_P1) blur_parallel.cc
	$(CXX) $(CXXFLAGS) -o blur_parallel blur_parallel.cc $(OBJS_P1) -lpthread

compare_bmp: $(OBJS_P1) compare_bmp.cc
	$(CXX) $(CXXFLAGS) -o compare_bmp compare_bmp.cc $(OBJS_P1)

# part 2
test_suite: $(TESTOBJS)  DoubleQueue.o
	$(CXX) $(CFLAGS) -o test_suite $(TESTOBJS) \
	$(CPPUNITFLAGS) DoubleQueue.o -lpthread

numbers: $(OBJS_P2)
	$(CXX) $(CXXFLAGS) -o numbers $(OBJS_P2) -lpthread

sequential_numbers: sequential_numbers.cc
	$(CXX) $(CXXFLAGS) -o sequential_numbers sequential_numbers.cc

# generic
%.o: %.cc $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -pthread

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -pthread

clean:
	/bin/rm -f *.o *~ *.gcno *.gcda *.gcov $(EXECS)
