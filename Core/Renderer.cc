#include "Renderer.h"

#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>

#define MIN( a, b ) ( ( a < b) ? a : b )

Renderer::Renderer(){
    /* Empty constructor */
}

Renderer::~Renderer(){
    for (int i = 0; i < WIDTH; i++) {
        free(frame_buffer[i]);
    }
    free(frame_buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}
    

void Renderer::Initialize(int fullscreen, int R, int G, int B){
    int window_mode = SDL_WINDOW_RESIZABLE;

    frame_buffer = (unsigned char **) malloc(WIDTH * sizeof(unsigned char *));
    memset(frame_buffer, 0, WIDTH * sizeof(unsigned char *));

    for (int i = 0; i < WIDTH; i++) {
        frame_buffer[i] = (unsigned char *) malloc(HEIGHT * sizeof(unsigned char));
        memset(frame_buffer[i], 0, HEIGHT * sizeof(unsigned char));
    }
    
   	window = SDL_CreateWindow("Chip8", 
   							  SDL_WINDOWPOS_CENTERED, 
   							  SDL_WINDOWPOS_CENTERED, 
   							  WINDOW_WIDTH, 
   							  WINDOW_HEIGHT, 
   							  window_mode);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    /* Set to fullscreen mode if flag present */
    if (fullscreen) { 
        ToggleFullscreen();
    }
   
    this->R = R;
    this->G = G;
    this->B = B;
}

void Renderer::UpdateRenderSpace() {

    SDL_DestroyRenderer(renderer); 
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	/* Get the current window size */	
	SDL_GetWindowSize(window, &WINDOW_WIDTH, &WINDOW_HEIGHT);
	
	int ratio_w = (WINDOW_WIDTH / WIDTH);
	int ratio_h = (WINDOW_HEIGHT / HEIGHT);

	SCALE = MIN(ratio_w, ratio_h);

	RENDER_WIDTH =  WIDTH * SCALE;
	RENDER_HEIGHT = HEIGHT * SCALE;

	RENDER_OFFSET_W = (WINDOW_WIDTH - RENDER_WIDTH);
	RENDER_OFFSET_H = (WINDOW_HEIGHT - RENDER_HEIGHT);
	if (RENDER_OFFSET_W > 0) {
		RENDER_OFFSET_W /= 2;
	}
	if (RENDER_OFFSET_H > 0) {
		RENDER_OFFSET_H /= 2;
	}

    /* Render WIDTH x HEIGHT should always be the greatest multiple 
    of 64 x 32 that fits in the window */
	fprintf(stderr, "%d(%d) X %d(%d)\n", WINDOW_WIDTH, RENDER_WIDTH, WINDOW_HEIGHT, RENDER_HEIGHT);
    RenderFrame(frame_buffer);
}

void Renderer::ToggleFullscreen() {

    if (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP) {

        /* Set Windowed */
        SDL_SetWindowFullscreen(window, 0);
        SDL_ShowCursor(SDL_ENABLE);

    } else {
        
        /* Set Fullscreen */
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        SDL_ShowCursor(SDL_DISABLE);
    }
}

void Renderer::RenderFrame(unsigned char **vram){
    /* Clear the screen */
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    /* Render the vram */
    SDL_Rect rectangle; 
    rectangle.x = 0;
    rectangle.y = 0;
    rectangle.w = SCALE;
    rectangle.h = SCALE;

    SDL_SetRenderDrawColor(renderer, R, G, B, 0);

    for (int i = 0; i < WIDTH; i++){
    	for (int j = 0; j < HEIGHT; j++){

    		rectangle.x = (i * SCALE) + RENDER_OFFSET_W;
	        rectangle.y = (j * SCALE) + RENDER_OFFSET_H;

            frame_buffer[i][j] = vram[i][j];
	  		
	  		if (vram[i][j]) {

                /* Render Foreground */
	    		SDL_SetRenderDrawColor(renderer, R, G, B, 255);
		        SDL_RenderFillRect(renderer, &rectangle);
	        } 
    	}
    } 
    /* Draw */
    SDL_RenderPresent(renderer);
}
