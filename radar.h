#ifndef RADAR_H
#define RADAR_H
#include "SDL.h"
#include <array>

class radar
{
    SDL_Renderer* renderer;
    SDL_Window* window;

    int centerX, centerY, diameter, winX, winY;

    std::array<float, 360> distances;
public:
    radar(SDL_Window* window, SDL_Renderer* renderer);
    ~radar();

    void findCenter();
    void empty();
    void addDistance(unsigned angle, float object);
    void draw();
};

#endif // RADAR_H
