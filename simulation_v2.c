#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

// Rozmiar okna
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 800

// Liczba cząsteczek
#define NUM_PARTICLES 100

// Domyślny promień cząsteczki
#define PARTICLE_RADIUS 10

// Maksymalna prędkość cząsteczki
#define MAX_SPEED 20.0f

// Stała grawitacyjna
#define GRAVITY 0.5f

//Stala zwalniajaca
#define DAMPING 0.5f

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

// Funkcja do pokazywana predkosci
void showSpeed(Particle *p) {
    printf("Particle speed: %.2f\n", getSpeed(p));
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
    float dx = b->x - a->x;
    float dy = b->y - a->y;
    float distance = sqrtf(dx * dx + dy * dy);
    return distance < (a->radius + b->radius);
}

// Funkcja do zmiany koloru cząsteczki
void changeColor(Particle *p) {
    p->r = rand() % 256;
    p->g = rand() % 256;
    p->b = rand() % 256;
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

    // a->dx = b->dx;
    // a->dy = b->dy;
    // b->dx = tempDx;
    // b->dy = tempDy;

    changeColor(a);
    changeColor(b);

    // a->radius += 0.1f;
    // b->radius += 0.1f;

    limitSpeed(a);
    limitSpeed(b);

    collisionsCount++;
    showSpeed(a);
    showSpeed(b);
}

// Funkcja do rysowania wykresu
void drawGraph(SDL_Renderer *renderer, float *data, int count) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    int graphWidth = WINDOW_WIDTH;
    int graphHeight = WINDOW_HEIGHT;
    int margin = 50; // Margines dla osi i etykiet

    // Rysowanie osi X i Y
    SDL_RenderDrawLine(renderer, margin, graphHeight - margin, graphWidth - margin, graphHeight - margin); // Oś X
    SDL_RenderDrawLine(renderer, margin, margin, margin, graphHeight - margin); // Oś Y

    // Rysowanie siatki pomocniczej
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    for (int i = margin; i < graphWidth - margin; i += 50) {
        SDL_RenderDrawLine(renderer, i, margin, i, graphHeight - margin);
    }
    for (int i = margin; i < graphHeight - margin; i += 50) {
        SDL_RenderDrawLine(renderer, margin, i, graphWidth - margin, i);
    }

    // Rysowanie linii wykresu
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Zmiana koloru linii wykresu na zielony
    for (int i = 1; i < count; i++) {
        int x1 = margin + (i - 1) * (graphWidth - 2 * margin) / count;
        int y1 = graphHeight - margin - (int)(data[i - 1] * 10);

        int x2 = margin + i * (graphWidth - 2 * margin) / count;
        int y2 = graphHeight - margin - (int)(data[i] * 10);

        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }

    // Dodanie etykiet do osi
    TTF_Font *font = TTF_OpenFont("/usr/share/fonts/fonts/ttf/JetBrainsMono-Bold.ttf", 16);
    if (font) {
        SDL_Color color = {255, 255, 255, 255};
        SDL_Surface *surface = TTF_RenderText_Solid(font, "TIME", color);
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect dstrect = {graphWidth / 2, graphHeight - margin + 10, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, NULL, &dstrect);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);

        surface = TTF_RenderText_Solid(font, "SPEED", color);
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        dstrect = (SDL_Rect){10, margin / 2, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, NULL, &dstrect);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);

        TTF_CloseFont(font);
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

    srand(time(NULL));

    Particle particles[NUM_PARTICLES];
    for (int i = 0; i < NUM_PARTICLES; i++) {
        particles[i].x = rand() % WINDOW_WIDTH;
        particles[i].y = WINDOW_HEIGHT / 2;
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

            if (particles[i].x - particles[i].radius < 0 || particles[i].x + particles[i].radius > WINDOW_WIDTH) {
                particles[i].dx = -particles[i].dx;
            }
            if (particles[i].y - particles[i].radius < 0 || particles[i].y + particles[i].radius > WINDOW_HEIGHT) {
                particles[i].dy = -particles[i].dy;
                particles[i].y = WINDOW_HEIGHT - particles[i].radius; // Upewnij się, że cząsteczka nie przekracza dolnej krawędzi
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

        // Check if any particle has a radius of 15 or more
        for (int i = 0; i < NUM_PARTICLES; i++) {
            if (particles[i].radius >= 50.0f) {
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
        "Wykres prędkości średnich",
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

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}