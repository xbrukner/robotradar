#include "radar.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <cmath>

#define RADAR_BACKGROUND 0xF1, 0xFF, 0xF7, 255
#define BACKGROUND 0x2C, 0x61, 0x43, 255
#define OBJECT_COLOR 0xB9, 0xFF, 0xD8, 255
#define DIRECTION_COLOR 0x29, 0x47, 0x56, 255

const double PI  = 3.141592653589793238463;
const double D2R = PI / 180.0;

Radar::Radar(SDL_Window* window, SDL_Renderer* renderer)
    : renderer(renderer), window(window), centerX(0), centerY(0), diameter(0), lastAngle(-1)
{
    distances.fill(1.0);
}

Radar::~Radar()
{
}

void Radar::findCenter() {
    SDL_GetWindowSize(window, &winX, &winY);

    centerX = winX / 2;
    centerY = winY / 2;

    diameter = std::min(centerX, centerY);
}

void Radar::drawBackground() {
    //Create radar background
    SDL_SetRenderDrawColor(renderer, RADAR_BACKGROUND);
    SDL_RenderClear(renderer);

    //Draw background around
    SDL_SetRenderDrawColor(renderer, BACKGROUND);

    for (int y = 0; y <= centerY; ++y) {
        if (y < centerY - diameter) { //Outside of radar - one line
            SDL_RenderDrawLine(renderer, 0, y, winX, y);
            SDL_RenderDrawLine(renderer, 0, winY - y, winX, winY - y);
        }
        else { //Inside of radar - two lines
            int edgeX = std::sqrt((diameter * diameter) - ((centerY - y) * (centerY - y)));
            SDL_RenderDrawLine(renderer, 0, y, centerX - edgeX, y);
            SDL_RenderDrawLine(renderer, centerX + edgeX, y, winX, y);
            SDL_RenderDrawLine(renderer, 0, winY - y, centerX - edgeX, winY - y);
            SDL_RenderDrawLine(renderer, centerX + edgeX, winY - y, winX, winY - y);
        }
    }
}

void Radar::addDistance(const Input& input) {
    distances[input.angle] = input.distance;
    lastAngle = input.angle;
    drawBackground();
    drawDistances();
    present();
}

void Radar::drawDistances() {
    for (unsigned i = 0; i < 360; ++i) {
        if (distances[i] < 1.0) { //There is an object on this angle
            float sinA = sin(i * D2R);
            float cosA = cos(i * D2R);

            //Draw only from this point to the edge, as center is already filled with background
            //Y is inverted
            SDL_SetRenderDrawColor(renderer, OBJECT_COLOR);
            SDL_RenderDrawLine(renderer,
                               centerX + diameter * cosA * distances[i],
                               centerY - diameter * sinA * distances[i],
                               centerX + diameter * cosA,
                               centerY - diameter * sinA);

            //Draw also direction line
            if (i == lastAngle) {
                SDL_SetRenderDrawColor(renderer, DIRECTION_COLOR);
                SDL_RenderDrawLine(renderer,
                                   centerX,
                                   centerY,
                                   centerX + diameter * cosA * distances[i],
                                   centerY - diameter * sinA * distances[i]);
            }
        }
    }
}


void Radar::present() {
    SDL_RenderPresent(renderer);
}

void Radar::redraw() {
    drawBackground();
    drawDistances();
    present();
}
