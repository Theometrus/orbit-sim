#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <stdio.h>
#include "nbody.h"


// Note, energy is not strictly conserved. This seems to be a product of how computers and the simulation work.
// As such, my validation criterion is that the energy in the system does not increase by more than 1%.

double runSimulation(char* filename, int numBodies, double dt, int niters, int nThreads) {
    threadPool* pool = init(filename, numBodies, dt, nThreads);
    double initialEnergy = calcEnergy(pool->data[0]->bodies, pool->data[0]->numBodies);
    while(niters > 0) {
        step(pool);
        niters--;
    }
    double finalEnergy = calcEnergy(pool->data[0]->bodies, pool->data[0]->numBodies);
    double percentIncrease = ((finalEnergy - initialEnergy)/initialEnergy) * 100;
    assert_true(percentIncrease < 1);
    return percentIncrease;
}

// NAMING CONVENTION: t<num threads>_d<dt>_<num iterations>_<file | random<num bodies>>

void t4_d1000_1000_file_test (void** state) {
    runSimulation("test/planets_testing.csv", 0, 1000, 1000, 4);
}

void t4_d100_1000_file_test (void** state) {
    runSimulation("test/planets_testing.csv", 0, 100, 1000, 4);
}

void t4_d100_100_file_test (void** state) {
    runSimulation("test/planets_testing.csv", 0, 100, 100, 4);
}

void t1_d100_100_file_test (void** state) {
    runSimulation("test/planets_testing.csv", 0, 100, 100, 1);
}

void t1_d1000_1000_file_test (void** state) {
    runSimulation("test/planets_testing.csv", 0, 1000, 1000, 1);
}

void t1_d100_1000_file_test (void** state) {
    runSimulation("test/planets_testing.csv", 0, 100, 1000, 1);
}

void t2_d10000_5000_random100_test (void** state) {
    runSimulation(NULL, 100, 10000, 5000, 4);
}

void t3_d118123_1247_random324_test (void** state) {
    runSimulation(NULL, 324, 118123, 1247, 3);
}

void t4_d1_1_random1_test (void** state) {
    runSimulation(NULL, 1, 1, 1, 4);
}

void t100_d100_100_random20_test (void** state) {
    runSimulation(NULL, 20, 100, 100, 100);
}

void invalid_num_bodies_test (void** state) {
    threadPool* pool = init(NULL, 0, 100, 3);
    assert_null(pool);
}

void invalid_dt_test (void** state) {
    threadPool* pool = init(NULL, 4, -1, 4);
    assert_null(pool);
}

void invalid_threads_test (void** state) {
    threadPool* pool = init(NULL, 4, 1, -1);
    assert_null(pool);
}

void invalid_multiple_test (void** state) {
    threadPool* pool = init(NULL, 0, 0, -1);
    assert_null(pool);
}

// Had to lower number of iterations for high body counts due to my machine being super slow
void t4_d100_300_random500_test (void** state) {
    runSimulation(NULL, 500, 100, 300, 4);
}

// Check that different thread numbers still give the same outcomes
void verify_same_result_simple_test (void** state) {
    double ret1 = runSimulation("test/planets_testing.csv", 0, 1000, 1000, 1);
    double ret4 = runSimulation("test/planets_testing.csv", 0, 1000, 1000, 4);
    assert_float_equal(ret1, ret4, 0.1);
}

void verify_same_result_complex_test (void** state) {
    double rets[6] = {
        runSimulation("test/planets_testing.csv", 0, 1000, 1000, 1), 
        runSimulation("test/planets_testing.csv", 0, 1000, 1000, 2),
        runSimulation("test/planets_testing.csv", 0, 1000, 1000, 3),
        runSimulation("test/planets_testing.csv", 0, 1000, 1000, 4),
        runSimulation("test/planets_testing.csv", 0, 1000, 1000, 5),
        runSimulation("test/planets_testing.csv", 0, 1000, 1000, 6)
    };

    for(int i = 0; i < 6; i++) {
        for(int j = i; j < 6; j++) {
            assert_float_equal(rets[i], rets[j], 0.1);
        }
    }
}

int setup (void ** state) {
    // Seed the random generator to have consistent 
    // random results for verification
    srand(4);
    return 0;
}

int teardown (void ** state) {
    return 0;
}


int main (void) {
    const struct CMUnitTest tests [] = {
        cmocka_unit_test (t4_d1000_1000_file_test),
        cmocka_unit_test (t4_d100_1000_file_test),
        cmocka_unit_test (t4_d100_100_file_test),
        cmocka_unit_test (t1_d1000_1000_file_test),
        cmocka_unit_test (t1_d100_1000_file_test),
        cmocka_unit_test (t1_d100_100_file_test),
        cmocka_unit_test (verify_same_result_simple_test),
        cmocka_unit_test (verify_same_result_complex_test),
        cmocka_unit_test (t2_d10000_5000_random100_test),
        cmocka_unit_test (t4_d100_300_random500_test),
        cmocka_unit_test (t3_d118123_1247_random324_test),
        cmocka_unit_test (t4_d1_1_random1_test),
        cmocka_unit_test (invalid_num_bodies_test),
        cmocka_unit_test (invalid_dt_test),
        cmocka_unit_test (invalid_threads_test),
        cmocka_unit_test (invalid_multiple_test),
        cmocka_unit_test (t100_d100_100_random20_test),
    };
    
    int count_fail_tests = cmocka_run_group_tests (tests, setup, teardown);

    return count_fail_tests;
}