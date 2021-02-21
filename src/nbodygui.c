#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include "nbody.h"

// Attempt to scale planet size in GUI according to mass
int getPlanetSizeGUI(double mass) {
	double ret = mass/10e23;
	if(ret > 10) { ret = 10; }
	if(ret < 1) { ret = 1; }
	return ret;
}

int main(int argc, char** argv) {	
	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;
	SDL_Event event;
	int finished = 0;

	if(argc < 8) {
		puts("Invalid number of arguments specified");
		exit(0);
	}

	int viewWidth = atoi(argv[1]);
	int viewHeight = atoi(argv[2]);
	int iterations = atoi(argv[3]);
	int dt = strtod(argv[4], NULL);
	int numBodies = 0;
	char* filePath = NULL;
	int numThreads = 1;

	if(strcmp(argv[5], "-b") == 0) {
		numBodies = atoi(argv[6]);
	} else if(strcmp(argv[5], "-f") == 0) {
		filePath = argv[6];
	} else {
		puts("Invalid arguments");
		exit(0);
	}

	double scale = strtod(argv[7], NULL);

	// Thread parameter specified
	if(argc > 8) {
		numThreads = atoi(argv[8]);
	}
	
	//Initialises SDL Video Acceleration
	//Check if return value is < 0
	SDL_Init(SDL_INIT_VIDEO);
	
	/**
	 * Creates a window to display
	 * Allows you to specify the window x,y position
	 * Allows you to specify the width and height of the window
	 * Option for flags for the window and potentially the different instances
	 * you may want to use with it. SHOWN, makes the window visible
	 * Should check if it returns NULL
	 */
	window = SDL_CreateWindow("SDL Template",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		viewWidth, viewHeight, SDL_WINDOW_SHOWN);

	if(window == NULL) { return -1; }
	
	/**
	 * Create Renderer retrieves the 2D rendering context of the window
	 * You will need to use this to use with SDL2_gfx library 
	 * This will give you access to the underlying rederer of the window
	 * Should check if it returns NULL
	 */
	renderer = SDL_CreateRenderer(window, -1,
		SDL_RENDERER_ACCELERATED 
		| SDL_RENDERER_PRESENTVSYNC);

	if(renderer == NULL) { return -1; }
	
	/**
	 * Render loop of your application
	 * You will perform your drawing in this loop here
	 */

	threadPool* pool = init(filePath, numBodies, dt, numThreads);
	body* bodies = pool->data[0]->bodies;
	numBodies = pool->data[0]->numBodies;

	int ctr = 0;
	while(ctr < iterations) {
		if (!finished) {ctr++;}
		//Sets the background colour 
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
		
		//Clears target with a specific drawing colour (prev function defines colour)
		SDL_RenderClear(renderer);

		for(int i = 0; i < numBodies; i++) {
			filledCircleColor(renderer, (bodies[i].x * scale) + viewWidth/2, (bodies[i].y * scale) + viewHeight/2, getPlanetSizeGUI(bodies[i].mass), 0xFF0000FF);
		}
		
		//Updates the screen with newly renderered image
		SDL_RenderPresent(renderer);
			
		step(pool);

		//Retrieves the events captured from SDL (just watching for windows close)
		if(SDL_PollEvent(&event)) {
			finished = (event.type == SDL_QUIT);
		}		
	}
	
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	cleanup(pool);
	return 0;	
}
