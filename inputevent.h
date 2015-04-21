#ifndef INPUTEVENT_H
#define INPUTEVENT_H
#include "SDL.h"
#include "input.h"
#include "radar.h"

class InputEvent
{
    int eventId;
public:
    InputEvent();
    ~InputEvent();

    void pushEvent(const Input& input);
    void handleEvent(const SDL_Event &event, Radar& Radar);

    SDL_Thread* demo();
    SDL_Thread* fileInput(const char* path);
};

#endif // INPUTEVENT_H
