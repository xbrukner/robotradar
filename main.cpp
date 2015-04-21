#include <iostream>
#include "SDL.h"
#include "radar.h"

using namespace std;

int main()
{
    SDL_Window* window;
    SDL_Renderer* renderer;

    // Initialize SDL.
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        return 1;

    // Create the window where we will draw.
    window = SDL_CreateWindow("Robot Radar",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              800, 640,
                              0);

    // We must call SDL_CreateRenderer in order for draw calls to affect this window.
    renderer = SDL_CreateRenderer(window, -1, 0);

    // Select the color for drawing. It is set to red here.
//    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

    // Clear the entire screen to our selected color.
//    SDL_RenderClear(renderer);
    radar r(window, renderer);
    r.findCenter();
    for (unsigned i = 0; i < 450; ++i) {
        r.empty();
        r.addDistance(i % 360, 0.8);
        r.draw();
        // Give us time to see the window.
        SDL_Delay(30);
    }

    // Always be sure to clean up
    SDL_Quit();
    return 0;
}

