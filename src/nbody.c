#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "nbody.h"

#define GCONST (6.67e-11) // Gravitational Constant

// Was supposed to use a macro but it wasn't working well
int NUMTHREADS = 1;

// Courtesy of https://stackoverflow.com/questions/54074517/how-do-i-generate-a-random-double-between-0-and-max-in-c
double getRandRange(double max) {
	return (double)rand() / (RAND_MAX / max);
}

struct Tuple {
	int a;
	body* b;
};

struct Tuple readFromFile(char* filename) {
	char buffer[256];
	int lineNo = 0;
	body* bodies = malloc(sizeof(body));

	FILE* file = fopen(filename, "r");
	while (fgets(buffer, 256, file)) {
		char* tmp = malloc(sizeof(buffer));
        strcpy(tmp, buffer);
		const char* tok;
		double values[7];
		int ctr = 0;

		// Following 6 lines courtesy of https://stackoverflow.com/questions/12911299/read-csv-file-in-c
        for (tok = strtok(buffer, ",");
            tok && *tok;
            tok = strtok(NULL, ",\n")) {
				values[ctr] = strtod(tok, NULL);
				ctr++;
			}

		if(lineNo != 0) {
			bodies = realloc(bodies, sizeof(body) * (lineNo + 1));
		}

		bodies[lineNo].x = values[0];
		bodies[lineNo].y = values[1];
		bodies[lineNo].z = values[2];

		bodies[lineNo].new_velocities = malloc(sizeof(double) * 3);

		bodies[lineNo].velocity_x = values[3], bodies[lineNo].new_velocities[0] = values[3];
		bodies[lineNo].velocity_y = values[4], bodies[lineNo].new_velocities[1] = values[4];
		bodies[lineNo].velocity_z = values[5], bodies[lineNo].new_velocities[2] = values[5];
		bodies[lineNo].mass = values[6];

		lineNo++;
        free(tmp);
    }

	fclose(file);
	struct Tuple ret = { lineNo, bodies };
	return ret;
}

body* genRandBodies(int numBodies) {
	body* bodies = malloc(sizeof(body) * numBodies);
	if(bodies == NULL) { return NULL; }

	// Max values can be tweaked here
	for(int i = 0; i < numBodies; i++) {
		bodies[i].x = getRandRange(10e12) - 5e12;
		bodies[i].y = getRandRange(10e12) - 5e12;
		bodies[i].z = getRandRange(10e12) - 5e12;

		bodies[i].new_velocities = malloc(sizeof(double) * 3);

		double vx = getRandRange(600000) - 300000;
		double vy = getRandRange(600000) - 300000;
		double vz = getRandRange(600000) - 300000;

		bodies[i].velocity_x = vx, bodies[i].new_velocities[0] = vx;
		bodies[i].velocity_y = vy, bodies[i].new_velocities[1] = vy;
		bodies[i].velocity_z = vz, bodies[i].new_velocities[2] = vz;
		bodies[i].mass = getRandRange(10e24);
	}
	return bodies;
}

void calcDiff(double* ret, body* a, body* b) {
	double dx = a->x - b->x;
	double dy = a->y - b->y;
	double dz = a->z - b->z;

	ret[0] = dx;
	ret[1] = dy;
	ret[2] = dz;
}

double calcDistance(double* diff) {
	if(diff == NULL) { return -1; }
	return sqrt(pow(diff[0], 2) + pow(diff[1], 2) + pow(diff[2], 2));
}

double calcMagnitude(double massA, double massB, double distance) {
	return ((GCONST * (massA * massB)) / pow(distance, 2));
}

void calcUnitVector(double* ret, double* diff, double distance) {
	for(int i = 0; i < 3; i++) { ret[i] = -1 * diff[i] / distance; }
}

void calcAcceleration(double* ret, double* unitVector, double magnitude, double mass) {
	for(int i = 0; i < 3; i++) { ret[i] = (unitVector[i] * magnitude) / mass; }
}

void* worker(void* tPool) {
	threadPool* pool = (threadPool*) tPool;
	workerData* data;
	double* unitVector = malloc(sizeof(double) * 3);
	double* accelerationVector = malloc(sizeof(double) * 3);
	double* diff = malloc(sizeof(double) * 3);

	while(1) {
		// Condition lock
		pthread_mutex_lock(pool->qLock);
		pool->threadsWaiting++;
		while(pool->remaining <= 0) {
			pthread_cond_wait(pool->qCond, pool->qLock);
		}

		// Terminating condition is a signal from the outside
		if(pool->done) { break; }

		// Critical section in the next 3 lines
		data = pool->data[pool->head];
		pool->head++;
		pool->remaining--;
		pthread_mutex_unlock(pool->qLock);

		// Worker thread works on its own section of the array from startIdx to endIdx
		int startIdx = data->startIdx;
		int endIdx = data->endIdx;
		int numBodies = data->numBodies;
		double dt = data->dt;
		body* bodies = data->bodies;
		
		int arrIdx = 0;

		for(int i = startIdx; i < endIdx; i++) {
			/*
			 * Register doubles used here to make use of temporal locality and
			 * to minimize false sharing (and to speed up calculations using registers). 
			 * New velocities are going to be recorded into them instead of being written 
			 * back to the shared array immediately
			 */
			register double tmpX = bodies[i].new_velocities[0];
			register double tmpY = bodies[i].new_velocities[1];
			register double tmpZ = bodies[i].new_velocities[2];

			for(int j = 0; j < numBodies; j++) {
				if(i == j) { continue; }

				calcDiff(diff, &bodies[i], &bodies[j]);

				double distance = calcDistance(diff);
				double magnitude = calcMagnitude(bodies[i].mass, bodies[j].mass, distance);

				calcUnitVector(unitVector, diff, distance);
				calcAcceleration(accelerationVector, unitVector, magnitude, bodies[i].mass);

				tmpX += (accelerationVector[0] * dt);
				tmpY += (accelerationVector[1] * dt);
				tmpZ += (accelerationVector[2] * dt);
			}

			// Even here, data is written back to the worker's struct in the end
			// rather than the bodies shared array to avoid false sharing
			*(data->result + (arrIdx * 3)) = tmpX;
			*(data->result + (arrIdx * 3) + 1) = tmpY;
			*(data->result + (arrIdx * 3) + 2) = tmpZ;
			arrIdx++;
		}

		// Since threadsDone is a shared variable, it needs to be incremented atomically
		// to avoid race conditions
		int ret = __atomic_add_fetch(&pool->threadsDone, 1, __ATOMIC_SEQ_CST);

		// Wake up manager
		if(ret >= NUMTHREADS) {
			pthread_mutex_lock(pool->qLock);
			pthread_cond_signal(pool->qEmpty);
			pthread_mutex_unlock(pool->qLock);
		}
	}

	// Cleanup
	free(unitVector);
	free(diff);
	free(accelerationVector);	
	return NULL;
}

// The first function to get called; the one that sets everything up.
// Number of bodies is ignored if a filename is specified. If filename is not specified, random bodies are generated.
threadPool* init(char* filename, int numBodies, double dt, int numThreads) {
	NUMTHREADS = numThreads;
	pthread_t threads[NUMTHREADS];
	body* bodies;
	threadPool* pool = malloc(sizeof(threadPool));
	workerData** data = malloc(sizeof(workerData*) * NUMTHREADS);
	pthread_cond_t* qCond = malloc(sizeof(pthread_cond_t));
	pthread_cond_t* qEmpty = malloc(sizeof(pthread_cond_t));
	pthread_mutex_t* qLock = malloc(sizeof(pthread_mutex_t));

	srand(time(NULL));

	if((dt < 0) | (numThreads < 1)) { return NULL; }

	pthread_mutex_init(qLock, NULL);
	pthread_cond_init(qCond, NULL);
	pthread_cond_init(qEmpty, NULL);

	if(filename != NULL) {
		struct Tuple t = readFromFile(filename);
		bodies = t.b;
		numBodies = t.a;
	} else {
		if(numBodies <= 0) { return NULL; }
		bodies = genRandBodies(numBodies);
	}

	/*
	 * The following for-loop attempts to distribute work equally among all
	 * threads. It does this by dynamically calculating the start and end indices
	 * for a given thread to work on. I didn't want to simply divide the number of
	 * bodies by the number of threads and call it a day. Instead, the calculation 
	 * tries to deal with situations such as 10 bodies and 4 threads. 
	 * By dividing 10/4 you'd get 2.5 bodies per thread, that's a distribution
	 * of either 2 2 2 4 or 3 3 3 1. Hardly ideal. The lastEndIdx calculations 
	 * attempt to distribute the work in a more 2 2 3 3 fashion in this case!
	 */
	int lastEndIdx = 0;
	for(int i = 0; i < NUMTHREADS; i++) {
		data[i] = malloc(sizeof(workerData));
		data[i]->bodies = bodies;
		data[i]->dt = dt;
		data[i]->numBodies = numBodies;
		data[i]->startIdx = lastEndIdx;
		lastEndIdx = (numBodies - lastEndIdx) / (NUMTHREADS - i) + lastEndIdx;
		data[i]->endIdx = lastEndIdx;
		data[i]->result = (double*) malloc(sizeof(double) * (data[i]->endIdx - data[i]->startIdx) * 3);
	}

	pool->data = data;
	pool->done = 0;
	pool->remaining = 0;
	pool->head = NUMTHREADS;
	pool->threadsDone = 0;
	pool->threadsWaiting = 0;
	pool->qCond = qCond;
	pool->qEmpty = qEmpty;
	pool->qLock = qLock;
	pool->size = NUMTHREADS;

	for(int i = 0; i < NUMTHREADS; i++) {
		// Threads are only created once as part of a thread pool. After they complete their task,
		// they hibernate until the manager (step function) wakes them up again.
		pthread_create(&threads[i], NULL, worker, (void*) pool);
	}
	return pool;
}

double calcEnergy(body* bodies, int numBodies) {
	double* diff = malloc(sizeof(double) * 3);
	double totalEnergy = 0;

	if(bodies == NULL) { return -1; }

	for(int i = 0; i < numBodies; i++) {
		double kineticEnergy = 0.5 * bodies[i].mass *
		 		(pow(bodies[i].velocity_x, 2) + pow(bodies[i].velocity_y, 2) + pow(bodies[i].velocity_z, 2));

		double potentialEnergy = 0;
		
		for(int j = i + 1; j < numBodies; j++) {
			calcDiff(diff, &bodies[i], &bodies[j]);
			potentialEnergy += (-1 * GCONST * bodies[i].mass * bodies[j].mass) / (calcDistance(diff));
		}

		totalEnergy += kineticEnergy - potentialEnergy;

	}

	free(diff);
	return totalEnergy;
}

void step(threadPool* pool) {
	if(pool == NULL) { return; }

	workerData** data = pool->data;
	body* bodies = data[0]->bodies;
	int numBodies = data[0]->numBodies;
	double dt = data[0]->dt;

	int expected = NUMTHREADS;

	// Waits for all threads to start waiting before proceeding
	while(!__atomic_compare_exchange_n(&pool->threadsWaiting, &expected, 0, 1, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
		expected = NUMTHREADS;
	}

	// Wake up workers
	pthread_mutex_lock(pool->qLock);
	pool->head = 0;
	pool->remaining = NUMTHREADS;
	pthread_cond_broadcast(pool->qCond);
	pthread_mutex_unlock(pool->qLock);

	// Wait for workers to finish
	pthread_mutex_lock(pool->qLock);
	while(pool->threadsDone < NUMTHREADS) {
		pthread_cond_wait(pool->qEmpty, pool->qLock);
	}

	pool->threadsDone = 0;
	pthread_mutex_unlock(pool->qLock);

	// Iterate through all the results and update the corresponding bodies'
	// NEW velocities (separate from current velocities)
	for(int i = 0; i < NUMTHREADS; i++) {
		int start = data[i]->startIdx;
		int end = data[i]->endIdx;
		for(int j = 0; j < end - start; j++) {
			bodies[start + j].new_velocities[0] = *(data[i]->result + (j * 3));
			bodies[start + j].new_velocities[1] = *(data[i]->result + (j * 3) + 1);
			bodies[start + j].new_velocities[2] = *(data[i]->result + (j * 3) + 2);
		}
	}

	// Update position of every body in the system. Set the new velocity to be the current velocity.
	for(int i = 0; i < numBodies; i++) {
		bodies[i].x = bodies[i].x + bodies[i].velocity_x * dt;
		bodies[i].y = bodies[i].y + bodies[i].velocity_y * dt;
		bodies[i].z = bodies[i].z + bodies[i].velocity_z * dt;

		bodies[i].velocity_x = bodies[i].new_velocities[0];
		bodies[i].velocity_y = bodies[i].new_velocities[1];
		bodies[i].velocity_z = bodies[i].new_velocities[2];
	}
}

// Free up memory
void cleanup(threadPool* pool) {
	workerData** data = pool->data;
	body* bodies = data[0]->bodies;
	free(pool->qCond);
	free(pool->qLock);
	free(pool->qEmpty);

	for(int i = 0; i < data[0]->numBodies; i++) {
		free(bodies[i].new_velocities);
	}

	free(bodies);

	for(int i = 0; i < NUMTHREADS; i++) {
		free(data[i]->result);
		free(data[i]);
	}
	free(data);
	free(pool);
}

