#include <SDL2/SDL.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define WINDOW_HEIGHT 800
#define WINDOW_WIDTH 1000

#define NUM_PARTICLES 2

typedef struct {
    float x,y; //Pozycja
    float dx, dy; //Kierunek ruchu w tym wypadku tylko po osi X
} Particle;

int main(){
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        printf("Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Particle simulation",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if(!window) {
        printf("Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Particle particles[NUM_PARTICLES];

    for(int i = 0; i < NUM_PARTICLES; i++){
        particles[i].x = rand() % WINDOW_WIDTH;
        particles[i].y = WINDOW_HEIGHT;
        particles[i].dx = (rand() % 2 == 0 ? 1 : -1) * (1 + rand() % 2);
        particles[i].dy = 0;
    }


    int running = 1;
    SDL_Event event;
    while(running){
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT){
                running = 0;
            }
        }

        for (int i = 0; i < NUM_PARTICLES; i++) {
            particles[i].x += particles[i].dx;
            particles[i].y += particles[i].dy;

            if (particles[i].x < 0 || particles[i].x > WINDOW_WIDTH) {
                particles[i].dx = -particles[i].dx;
            }
            if (particles[i].y < 0 || particles[i].y > WINDOW_HEIGHT) {
                particles[i].dy = -particles[i].dy;
                particles[i].y = WINDOW_HEIGHT; // Upewnij się, że cząsteczka nie przekracza dolnej krawędzi
            }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}