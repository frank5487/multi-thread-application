cmake_minimum_required(VERSION 3.24)
project(HW2)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(HW2
        blur_parallel.cc
        blur_sequential.cc
        compare_bmp.cc
        cqdbmp.c
        cqdbmp.h
        DoubleQueue.cc
        DoubleQueue.h
        negative.cc
        qdbmp.cc
        qdbmp.h
        sequential_numbers.cc
        test_doublequeue.cc
        test_suite.cc
        test_suite.h numbers.cc)
