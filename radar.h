#ifndef RADAR_H
#define RADAR_H
#include "SDL.h"
#include "input.h"
#include <array>

class Radar
{
    SDL_Renderer* renderer;
    SDL_Window* window;

    int centerX, centerY, diameter, winX, winY;

    int lastAngle;

    std::array<float, 360> distances;
public:
    Radar(SDL_Window* window, SDL_Renderer* renderer);
    ~Radar();

    void findCenter();
    void drawBackground();
    void addDistance(const Input& input);
    void present();
    void redraw();
    void drawDistances();
};

#endif // RADAR_H
