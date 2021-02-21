#include "nbody.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
	if(argc < 5) {
		puts("Invalid number of arguments specified");
		exit(0);
	}

	// Parse command line args
	int iterations = atoi(argv[1]);
	int dt = strtod(argv[2], NULL);
	int numBodies = 0;
	char* filePath = NULL;
	int numThreads = 1;

	if(strcmp(argv[3], "-b") == 0) {
		numBodies = atoi(argv[4]);
	} else if(strcmp(argv[3], "-f") == 0) {
		filePath = argv[4];
	} else {
		puts("Invalid arguments");
		exit(0);
	}

	// Thread parameter specified
	if(argc > 5) {
		numThreads = atoi(argv[5]);
	}

    threadPool* pool = init(filePath, numBodies, dt, numThreads);
	body* bodies = pool->data[0]->bodies;
	numBodies = pool->data[0]->numBodies;

	int ctr = 0;
    double initialEnergy = calcEnergy(bodies, numBodies);

	// Main loop
	while(ctr < iterations) {
        step(pool);
        ctr++;
    }

    double finalEnergy = calcEnergy(bodies, numBodies);

    printf("The initial energy was %.4e\n", initialEnergy);
    printf("The final energy was %.4e\n", finalEnergy);
    printf("The total change in energy throughout the simulation was %.4e\n", finalEnergy - initialEnergy);

	cleanup(pool);
}