#include "inputevent.h"
#include <cassert>

InputEvent::InputEvent()
    : eventId(SDL_RegisterEvents(1))
{
    assert(eventId >= 0);
}

InputEvent::~InputEvent()
{
}

void InputEvent::pushEvent(const Input &input) {
    //https://wiki.libsdl.org/SDL_UserEvent
    SDL_Event event;
    SDL_zero(event);
    event.type = eventId;
    event.user.code = 0;
    event.user.data1 = new Input(input);
    event.user.data2 = nullptr;
    SDL_PushEvent(&event);
}

void InputEvent::handleEvent(const SDL_Event &event, Radar& radar) {
    if (event.type == eventId) {
        Input* input = (Input*) event.user.data1;
        radar.addDistance(*input);
        delete input;
    }
}

int demoThread(void* data) {
    InputEvent* ie = (InputEvent*) data;

    for (unsigned i = 0; i < 450; ++i) {
        ie->pushEvent(Input(i % 360, 0.8));
        SDL_Delay(30);
    }

    return 0;
}

SDL_Thread* InputEvent::demo() {
    return SDL_CreateThread(demoThread, "DemoThread", this);
}
