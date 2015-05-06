#ifndef INPUTEVENT_H
#define INPUTEVENT_H
#include "SDL.h"
#include "input.h"
#include "radar.h"
#include <string>

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
    SDL_Thread* socketInput(const char** argv);

    SDL_mutex* mutex;
    std::string keys;
    void lock();
    void unlock();
    //Leaves lock open if returns true
    bool hasKey();
    void clear();
    void addKey(char c);
};

#endif // INPUTEVENT_H
