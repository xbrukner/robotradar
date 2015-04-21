#include <iostream>
#include "SDL.h"
#include "radar.h"
#include "inputevent.h"

using namespace std;

int main(int argc, char** argv)
{
    SDL_Window* window;
    SDL_Renderer* renderer;

    if (argc != 2) {
        std::cout << "Usage: robotradar pipe" << std::endl;
        return 0;
    }

    // Initialize SDL.
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        return 1;

    // Create the window where we will draw.
    window = SDL_CreateWindow("Robot Radar",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              800, 640,
                              SDL_WINDOW_RESIZABLE);

    // We must call SDL_CreateRenderer in order for draw calls to affect this window.
    renderer = SDL_CreateRenderer(window, -1, 0);

    //Create Radar
    Radar r(window, renderer);
    r.findCenter();
    r.redraw();

    //Create InputEvent
    InputEvent ie;
    //ie.demo();
    ie.fileInput(argv[1]);

    bool quit = false;
    while (!quit) {
        SDL_Event event;
        SDL_WaitEvent(&event);
        switch (event.type) {
        case SDL_QUIT:
            quit = true;
            break;
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                r.findCenter();
                r.redraw();
            }
            break;
        default:
            ie.handleEvent(event, r);
            break;
        }
    }

    // Always be sure to clean up
    SDL_Quit();
    return 0;
}

