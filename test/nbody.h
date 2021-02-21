#ifndef NBODY_H
#define NBODY_H
#include <pthread.h>

// Comment in report about how old struct was bigger than a cache line!!
typedef struct body {
	double x;
	double y;
	double z;
	double velocity_x;
	double velocity_y;
	double velocity_z;
	double* new_velocities;
	double mass;
} body;

typedef struct workerData {
	int startIdx;
	int endIdx;
	body* bodies;
	int numBodies;
	double dt;
	double* result;
	// To further combat false sharing by separating structs to different cache lines
	// (assuming a cache line is 64bytes in this case)
	char padding[20];
} workerData;

typedef struct threadPool {
	pthread_mutex_t* qLock;
	pthread_cond_t* qCond;
	pthread_cond_t* qEmpty;
	workerData** data;
	int threadsDone;
	int threadsWaiting;
	int remaining;
	int head;
	int size;
	int done;
} threadPool;

void step(threadPool* pool);
threadPool* init(char* filename, int numBodies, double dt, int numThreads);
double calcEnergy(body* bodies, int numBodies);
void cleanup(threadPool* pool);

#endif
