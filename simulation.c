//KOMENDA DO KOMPILACJI PLIKU: gcc -o simulation simulation.c -lSDL2 -lm

#include <SDL2/SDL.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

// Rozmiar okna
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 800

// Liczba cząsteczek
#define NUM_PARTICLES 100

// Promień cząsteczki
#define PARTICLE_RADIUS 20

// Maksymalna prędkość cząsteczki
#define MAX_SPEED 20.0f

// Struktura cząsteczki
typedef struct {
    float x, y;       // Pozycja
    float dx, dy;     // Prędkość (kierunek ruchu)
    Uint8 r, g, b;    // Kolor (RGB)
} Particle;

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
    return dist < (2 * PARTICLE_RADIUS);
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
    // Oblicz różnicę pozycji
    float dx = b->x - a->x;
    float dy = b->y - a->y;
    float distance = sqrtf(dx * dx + dy * dy);

    // Oblicz przesunięcie w celu separacji
    float overlap = (2 * PARTICLE_RADIUS - distance) / 2.0f;
    float unitDx = dx / distance;//Zastosowanie normalizacji w celu uzyskania kierunku cząsteczki bez zmiany proporcji
    float unitDy = dy / distance;

    // Przesuń cząsteczki na zewnątrz
    a->x -= unitDx * overlap;
    a->y -= unitDy * overlap;
    b->x += unitDx * overlap;
    b->y += unitDy * overlap;

    // Zamiana kierunków prędkości (odbijanie)
    float tempDx = a->dx;
    float tempDy = a->dy;
    a->dx = b->dx;
    a->dy = b->dy;
    b->dx = tempDx;
    b->dy = tempDy;

    // Zmiana kolorów
    changeColor(a);
    changeColor(b);

    // Ograniczenie prędkości
    limitSpeed(a);
    limitSpeed(b);
}

int main() {
    // Inicjalizacja SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Błąd inicjalizacji SDL: %s\n", SDL_GetError());
        return 1;
    }

    // Utworzenie okna
    SDL_Window *window = SDL_CreateWindow(
        "Symulacja wielu cząsteczek z kolizjami, FPS:60",
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

    // Utworzenie renderera
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Błąd tworzenia renderera: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Inicjalizacja generatora liczb losowych
    srand(time(NULL));

    // Tworzenie cząsteczek
    Particle particles[NUM_PARTICLES];
    for (int i = 0; i < NUM_PARTICLES; i++) {
        particles[i].x = rand() % (WINDOW_WIDTH - 2 * PARTICLE_RADIUS) + PARTICLE_RADIUS;
        particles[i].y = rand() % (WINDOW_HEIGHT - 2 * PARTICLE_RADIUS) + PARTICLE_RADIUS;
        particles[i].dx = (rand() % 2 == 0 ? 1 : -1) * (1 + rand() % 2);
        particles[i].dy = (rand() % 2 == 0 ? 1 : -1) * (1 + rand() % 2);
        particles[i].r = rand() % 200;
        particles[i].g = rand() % 200;
        particles[i].b = rand() % 200;
        limitSpeed(&particles[i]);
    }

    // Flaga działającego programu
    int running = 1;
    SDL_Event event;

    while (running) {
        // Obsługa zdarzeń
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        // Aktualizacja pozycji cząsteczek
        for (int i = 0; i < NUM_PARTICLES; i++) {
            particles[i].x += particles[i].dx;
            particles[i].y += particles[i].dy;

            // Odbicie od lewej i prawej ściany
            if (particles[i].x - PARTICLE_RADIUS < 0) {
                particles[i].x = PARTICLE_RADIUS; // Korekta pozycji
                particles[i].dx = -particles[i].dx;
            } else if (particles[i].x + PARTICLE_RADIUS > WINDOW_WIDTH) {
                particles[i].x = WINDOW_WIDTH - PARTICLE_RADIUS; // Korekta pozycji
                particles[i].dx = -particles[i].dx;
            }

            // Odbicie od górnej i dolnej ściany
            if (particles[i].y - PARTICLE_RADIUS < 0) {
                particles[i].y = PARTICLE_RADIUS; // Korekta pozycji
                particles[i].dy = -particles[i].dy;
            } else if (particles[i].y + PARTICLE_RADIUS > WINDOW_HEIGHT) {
                particles[i].y = WINDOW_HEIGHT - PARTICLE_RADIUS; // Korekta pozycji
                particles[i].dy = -particles[i].dy;
            }
        }


        // Obsługa kolizji między cząsteczkami
        for (int i = 0; i < NUM_PARTICLES; i++) {
            for (int j = i + 1; j < NUM_PARTICLES; j++) {
                if (checkCollision(&particles[i], &particles[j])) {
                    handleCollision(&particles[i], &particles[j]);
                }
            }
        }

        // Wyczyść ekran
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Czarny
        SDL_RenderClear(renderer);

        // Rysowanie cząsteczek
        for (int i = 0; i < NUM_PARTICLES; i++) {
            SDL_SetRenderDrawColor(renderer, particles[i].r, particles[i].g, particles[i].b, 255);
            drawCircle(renderer, (int)particles[i].x, (int)particles[i].y, PARTICLE_RADIUS);
        }

        // Wyświetlenie nowej klatki
        SDL_RenderPresent(renderer);

        // Opóźnienie
        SDL_Delay(16); // Około 60 FPS
    }

    // Sprzątanie zasobów SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

