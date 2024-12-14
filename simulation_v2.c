#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define NUM_PARTICLES 100
#define MAX_SPEED 5.0f
#define CIRCLE_CENTER_X (WINDOW_WIDTH / 2)
#define CIRCLE_CENTER_Y (WINDOW_HEIGHT / 2)
#define CIRCLE_RADIUS 200

typedef struct {
    float x, y;
    float dx, dy;
    float radius;
    Uint8 r, g, b;
} Particle;

// Declare a global variable for collision count
int collisionCount = 0;

// Function prototypes
void changeColor(Particle *p);
void limitSpeed(Particle *p);
void renderText(SDL_Renderer *renderer, const char *text, int x, int y);
void handleCollision(Particle *a, Particle *b);
int checkCollision(Particle *a, Particle *b);
void drawCircle(SDL_Renderer *renderer, int x, int y, int radius);

// Function to change the color of a particle
void changeColor(Particle *p) {
    p->r = rand() % 256;
    p->g = rand() % 256;
    p->b = rand() % 256;
}

// Function to limit the speed of a particle
void limitSpeed(Particle *p) {
    float speed = sqrtf(p->dx * p->dx + p->dy * p->dy);
    if (speed > MAX_SPEED) {
        p->dx = (p->dx / speed) * MAX_SPEED;
        p->dy = (p->dy / speed) * MAX_SPEED;
    }
}

// Function to render text on the screen
void renderText(SDL_Renderer *renderer, const char *text, int x, int y) {
    TTF_Font *font = TTF_OpenFont("arial.ttf", 24);
    if (!font) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return;
    }

    SDL_Color color = {255, 255, 255, 255};
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect dstrect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dstrect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    TTF_CloseFont(font);
}

// Function to handle collision with separation
void handleCollision(Particle *a, Particle *b) {
    float dx = b->x - a->x;
    float dy = b->y - a->y;
    float distance = sqrtf(dx * dx + dy * dy);

    float overlap = (a->radius + b->radius - distance) / 2.0f;
    float unitDx = dx / distance;
    float unitDy = dy / distance;

    a->x -= unitDx * overlap;
    a->y -= unitDy * overlap;
    b->x += unitDx * overlap;
    b->y += unitDy * overlap;

    float tempDx = a->dx;
    float tempDy = a->dy;
    a->dx = b->dx;
    a->dy = b->dy;
    b->dx = tempDx;
    b->dy = tempDy;

    changeColor(a);
    changeColor(b);

    // Increase particle radii
    a->radius += 0.1f;
    b->radius += 0.1f;

    limitSpeed(a);
    limitSpeed(b);

    // Increment the collision count
    collisionCount++;
}

// Function to check if two particles are colliding
int checkCollision(Particle *a, Particle *b) {
    float dx = b->x - a->x;
    float dy = b->y - a->y;
    float distance = sqrtf(dx * dx + dy * dy);
    return distance < (a->radius + b->radius);
}

// Function to draw a circle
void drawCircle(SDL_Renderer *renderer, int x, int y, int radius) {
    int offsetX, offsetY, d;
    offsetX = 0;
    offsetY = radius;
    d = radius - 1;
    while (offsetY >= offsetX) {
        SDL_RenderDrawPoint(renderer, x + offsetX, y + offsetY);
        SDL_RenderDrawPoint(renderer, x + offsetY, y + offsetX);
        SDL_RenderDrawPoint(renderer, x - offsetX, y + offsetY);
        SDL_RenderDrawPoint(renderer, x - offsetY, y + offsetX);
        SDL_RenderDrawPoint(renderer, x + offsetX, y - offsetY);
        SDL_RenderDrawPoint(renderer, x + offsetY, y - offsetX);
        SDL_RenderDrawPoint(renderer, x - offsetX, y - offsetY);
        SDL_RenderDrawPoint(renderer, x - offsetY, y - offsetX);
        if (d >= 2 * offsetX) {
            d -= 2 * offsetX + 1;
            offsetX += 1;
        } else if (d < 2 * (radius - offsetY)) {
            d += 2 * offsetY - 1;
            offsetY -= 1;
        } else {
            d += 2 * (offsetY - offsetX - 1);
            offsetY -= 1;
            offsetX += 1;
        }
    }
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Błąd inicjalizacji SDL: %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() == -1) {
        printf("Błąd inicjalizacji SDL_ttf: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Symulacja wielu cząsteczek z ograniczeniem okręgu",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        printf("Błąd tworzenia okna: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Błąd tworzenia renderera: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Particle particles[NUM_PARTICLES];
    for (int i = 0; i < NUM_PARTICLES; i++) {
        particles[i].x = rand() % WINDOW_WIDTH;
        particles[i].y = rand() % WINDOW_HEIGHT;
        particles[i].dx = (float)(rand() % 200 - 100) / 100.0f;
        particles[i].dy = (float)(rand() % 200 - 100) / 100.0f;
        particles[i].radius = 5.0f;
        particles[i].r = rand() % 256;
        particles[i].g = rand() % 256;
        particles[i].b = rand() % 256;
    }

    int running = 1;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        for (int i = 0; i < NUM_PARTICLES; i++) {
            particles[i].x += particles[i].dx;
            particles[i].y += particles[i].dy;

            if (particles[i].x - particles[i].radius < 0 || particles[i].x + particles[i].radius > WINDOW_WIDTH) {
                particles[i].dx = -particles[i].dx;
            }
            if (particles[i].y - particles[i].radius < 0 || particles[i].y + particles[i].radius > WINDOW_HEIGHT) {
                particles[i].dy = -particles[i].dy;
            }

            limitSpeed(&particles[i]);
        }

        for (int i = 0; i < NUM_PARTICLES; i++) {
            for (int j = i + 1; j < NUM_PARTICLES; j++) {
                if (checkCollision(&particles[i], &particles[j])) {
                    handleCollision(&particles[i], &particles[j]);
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        drawCircle(renderer, CIRCLE_CENTER_X, CIRCLE_CENTER_Y, CIRCLE_RADIUS);

        for (int i = 0; i < NUM_PARTICLES; i++) {
            SDL_SetRenderDrawColor(renderer, particles[i].r, particles[i].g, particles[i].b, 255);
            drawCircle(renderer, (int)particles[i].x, (int)particles[i].y, (int)particles[i].radius);
        }

        // Render the collision count
        char collisionText[50];
        sprintf(collisionText, "Liczba kolizji: %d", collisionCount);
        renderText(renderer, collisionText, 10, 10);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}