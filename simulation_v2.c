// KOMENDA DO KOMPILACJI PLIKU: gcc -o simulation simulation.c -lSDL2 -lm

#include <SDL2/SDL.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

// Rozmiar okna
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 800

// Liczba cząsteczek
#define NUM_PARTICLES 20

// Domyślny promień cząsteczki
#define PARTICLE_RADIUS 10

// Maksymalna prędkość cząsteczki
#define MAX_SPEED 20.0f

// Parametry okręgu ograniczającego
#define CIRCLE_CENTER_X (WINDOW_WIDTH / 2)
#define CIRCLE_CENTER_Y (WINDOW_HEIGHT / 2)
#define CIRCLE_RADIUS 350

// Stała grawitacyjna
#define GRAVITY 0.5f

//Stala zwalniajaca
#define DAMPING 0.99f

// Licznik kolizji
int collisionsCount = 0;

// Struktura cząsteczki
typedef struct {
    float x, y;       // Pozycja
    float dx, dy;     // Prędkość (kierunek ruchu)
    Uint8 r, g, b;    // Kolor (RGB)
    float radius;     // Dynamiczny promień
} Particle;

// Dane do wykresu
#define MAX_POINTS 1000
float avgSpeeds[MAX_POINTS];
int pointCount = 0;

// Funkcja do pokazywania prędkości cząsteczki
float getSpeed(Particle *p) {
    return sqrtf(p->dx * p->dx + p->dy * p->dy);
}

// Funkcja do rysowania okręgu
void drawCircle(SDL_Renderer *renderer, int x, int y, int radius) {
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w;
            int dy = radius - h;
            if ((dx * dx + dy * dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, x + dx, y + dy);
            }
        }
    }
}

// Funkcja do wykrywania kolizji między dwiema cząsteczkami
int checkCollision(Particle *a, Particle *b) {
    float dist = sqrtf((a->x - b->x) * (a->x - b->x) + (a->y - b->y) * (a->y - b->y));
    return dist < (a->radius + b->radius);
}

// Funkcja do zmiany koloru cząsteczki na losowy
void changeColor(Particle *p) {
    p->r = rand() % 200;
    p->g = rand() % 200;
    p->b = rand() % 200;
}

// Funkcja ograniczająca prędkość
void limitSpeed(Particle *p) {
    float speed = sqrtf(p->dx * p->dx + p->dy * p->dy);
    if (speed > MAX_SPEED) {
        p->dx = (p->dx / speed) * MAX_SPEED;
        p->dy = (p->dy / speed) * MAX_SPEED;
    }
}

// Funkcja do obsługi kolizji z separacją
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
    a->dx = b->dx * DAMPING;
    a->dy = b->dy * DAMPING;
    b->dx = tempDx * DAMPING;
    b->dy = tempDy * DAMPING;

    changeColor(a);
    changeColor(b);

    a->radius += 0.1f;
    b->radius += 0.1f;

    limitSpeed(a);
    limitSpeed(b);

    collisionsCount++;
}

// Funkcja do rysowania wykresu
void drawGraph(SDL_Renderer *renderer, float *data, int count) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    int graphWidth = WINDOW_WIDTH;
    int graphHeight = WINDOW_HEIGHT;

    for (int i = 1; i < count; i++) {
        int x1 = (i - 1) * (graphWidth / count);
        int y1 = graphHeight - (int)(data[i - 1] * 10);

        int x2 = i * (graphWidth / count);
        int y2 = graphHeight - (int)(data[i] * 10);

        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Błąd inicjalizacji SDL: %s\n", SDL_GetError());
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

    srand(time(NULL));

    Particle particles[NUM_PARTICLES];
    for (int i = 0; i < NUM_PARTICLES; i++) {
        particles[i].x = CIRCLE_CENTER_X;
        particles[i].y = CIRCLE_CENTER_Y - (CIRCLE_RADIUS + 2 * PARTICLE_RADIUS);
        particles[i].dx = (rand() % 2 == 0 ? 1 : -1) * (1 + rand() % 2);
        particles[i].dy = (rand() % 2 == 0 ? 1 : -1) * (1 + rand() % 2);
        particles[i].r = rand() % 200;
        particles[i].g = rand() % 200;
        particles[i].b = rand() % 200;
        particles[i].radius = PARTICLE_RADIUS;
        limitSpeed(&particles[i]);
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
            particles[i].dy += GRAVITY;

            particles[i].x += particles[i].dx;
            particles[i].y += particles[i].dy;

            float distX = particles[i].x - CIRCLE_CENTER_X;
            float distY = particles[i].y - CIRCLE_CENTER_Y;
            float distance = sqrtf(distX * distX + distY * distY);

            if (distance + particles[i].radius > CIRCLE_RADIUS) {
                float overlap = (distance + particles[i].radius) - CIRCLE_RADIUS;
                float unitX = distX / distance;
                float unitY = distY / distance;
                particles[i].x -= unitX * overlap;
                particles[i].y -= unitY * overlap;

                float dotProduct = particles[i].dx * unitX + particles[i].dy * unitY;
                particles[i].dx -= 2 * dotProduct * unitX;
                particles[i].dy -= 2 * dotProduct * unitY;

                limitSpeed(&particles[i]);
            }
        }

        for (int i = 0; i < NUM_PARTICLES; i++) {
            for (int j = i + 1; j < NUM_PARTICLES; j++) {
                if (checkCollision(&particles[i], &particles[j])) {
                    handleCollision(&particles[i], &particles[j]);
                }
            }
        }

        for (int i = 0; i < NUM_PARTICLES; i++){
            if(particles[i].radius > 100.0f) {
                running = 0;
                break;
            }
        }

        float avgSpeed = 0;
        for (int i = 0; i < NUM_PARTICLES; i++) {
            avgSpeed += getSpeed(&particles[i]);
        }
        avgSpeed /= NUM_PARTICLES;

        if (pointCount < MAX_POINTS) {
            avgSpeeds[pointCount++] = avgSpeed;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        drawCircle(renderer, CIRCLE_CENTER_X, CIRCLE_CENTER_Y, CIRCLE_RADIUS);

        for (int i = 0; i < NUM_PARTICLES; i++) {
            SDL_SetRenderDrawColor(renderer, particles[i].r, particles[i].g, particles[i].b, 255);
            drawCircle(renderer, (int)particles[i].x, (int)particles[i].y, (int)particles[i].radius);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Window *graphWindow = SDL_CreateWindow(
        "Wykres średniej prędkości",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    SDL_Renderer *graphRenderer = SDL_CreateRenderer(graphWindow, -1, SDL_RENDERER_ACCELERATED);

    int graphRunning = 1;
    while (graphRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                graphRunning = 0;
            }
        }

        SDL_SetRenderDrawColor(graphRenderer, 0, 0, 0, 255);
        SDL_RenderClear(graphRenderer);

        drawGraph(graphRenderer, avgSpeeds, pointCount);

        SDL_RenderPresent(graphRenderer);
    }

    SDL_DestroyRenderer(graphRenderer);
    SDL_DestroyWindow(graphWindow);

    SDL_Quit();

    return 0;
}
