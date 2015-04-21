#include "inputevent.h"
#include <cassert>
#include <iostream>
#include <string>
#include <sstream>

extern "C" {
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>
}

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

typedef std::pair<InputEvent*, const char*> fileType;
int fileThread(void* data) {
    fileType* in = (fileType*) data;

    InputEvent* ie = in->first;
    const char* path = in->second;

    int pipe = open(path, O_RDONLY, O_NONBLOCK, 0);

    if (pipe == -1) {
        perror("pipe");
        return 0;
    }

    fd_set fds;
    FD_ZERO(&fds);

    while (true) {
        char input[20];

        //Wait for input
        FD_SET(pipe, &fds);

        int s = select(pipe + 1, &fds, NULL, NULL, NULL);
        if (s < 0) {
            perror("select");
            return 0;
        }

        int r = read(pipe, input, 20);
        if (r > 0) {
            std::string s(input);
            std::stringstream ss(s);
            int angle;
            float distance;

            ss >> angle >> distance;
            if (ss.good()) {
                ie->pushEvent(Input(angle, distance));
            }
        }
        if (r < 0) {
            perror("read");
            return 0;
        }
    }

    return 0;
}

SDL_Thread* InputEvent::demo() {
    return SDL_CreateThread(demoThread, "DemoThread", this);
}

SDL_Thread* InputEvent::fileInput(const char *path) {
    return SDL_CreateThread(fileThread, "FileThread", new fileType(this, path));
}
