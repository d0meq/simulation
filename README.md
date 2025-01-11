### SYMULACJA RUCHU

## Nagłówki i definicje

```
#include <SDL2/SDL.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
```

Nagłówki te dołączają biblioteki takie jak:
1. **SDL2** - do obsługi grafiki
2. **math.h** - pozwala używać funkcji matematycznych 
3. **stdlib.h** - zawiera funkcje do generowania liczb losowych i zarządzania pamięcią
4. **time.h** - służy do pracy z czasem potrzebna do inicjalizacji generatora liczb losowych

## Ustawienia programu

```
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 800
#define NUM_PARTICLES 100
#define PARTICLE_RADIUS 20
#define MAX_SPEED 20.0f
```

Są to zmienne globalne które odpowiadają za:
1. **WINDOW_WIDTH** i **WINDOW_HEIGHT** - rozmiary okna symulacji
2. **NUM_PARTICLES** - liczba cząsteczek w symulacji
3. **PARTICLE RADIUS** - promien każdej cząsteczki
4. **MAX_SPEED** - maksymalna prędkość z jaką kulki mogą się poruszać

## STRUKTURA CZĄSTECZKI
```
typedef struct {
    float x, y;       // Pozycja cząsteczki
    float dx, dy;     // Prędkość (ruch w kierunku x i y)
    Uint8 r, g, b;    // Kolor cząsteczki (RGB)
} Particle;
```

Jest to struktura pozwalająca na określenie jakie parametry ma przypisana każda cząsteczka:

1. **x, y** - pozycja na ekranie
2. **dx, dy** - prędkość w poziomie i pionie
3. **r, g, b** - kolor cząsteczki w formacie RGB

## FUNKCJE POMOCNICZE

```
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
```

Funkcja **drawCircle()** opowiada za narysowanie kółka na ekranie (aktualnie nie jest używana):

1. Przesuwa się przez kwadrat o wymiarach równych średnicy kółka
2. Jeśli punkt **(dx, dy)** znajduje się wewnątrz okręgu (wykorzystując równanie: **dx^2 + dy^2 <= r^2**), rysuje go na ekranie

```
int checkCollision(Particle *a, Particle *b) {
    float dist = sqrtf((a->x - b->x) * (a->x - b->x) + (a->y - b->y) * (a->y - b->y));
    return dist < (2 * PARTICLE_RADIUS);
}
```

Funkcja **checkCollision** odpowiada za sprawdzenie czy cząsteczki uderzają o siebie:

1. Oblicza odległość między dwiema cząsteczkami (a i b) za pomocą wzoru Pitagorasa
2. Sprawdza, czy odległość jest mniejsza niż dwa razy promień (czyli czy cząsteczki się dotykają)

```
void changeColor(Particle *p) {
    p->r = rand() % 200;
    p->g = rand() % 200;
    p->b = rand() % 200;
}
```

Funkcja **changeColor()** odpowiada za zmiane koloru cząsteczek:

1. Ustawia losowy kolor dla cząsteczki w zakresie od 0 do 199
