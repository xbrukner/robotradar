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

#include <sys/socket.h>
#include <netdb.h>
#include <err.h>
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

typedef std::pair<InputEvent*, const char**> socketType;
int socketThread(void* data) {
    socketType* in = (socketType*) data;

    InputEvent* ie = in->first;
    const char** argv = in->second;

    //getaddrinfo(3)
    struct addrinfo hints, *res, *res0;
    int error;
    int s;
    const char *cause = NULL;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    error = getaddrinfo(argv[1], argv[2], &hints, &res0);
    if (error) {
        errx(1, "%s", gai_strerror(error));
        /*NOTREACHED*/
    }
    s = -1;
    for (res = res0; res; res = res->ai_next) {
        s = socket(res->ai_family, res->ai_socktype,
                   res->ai_protocol);
        if (s < 0) {
            cause = "socket";
            continue;
        }

        if (connect(s, res->ai_addr, res->ai_addrlen) < 0) {
            cause = "connect";
            close(s);
            s = -1;
            continue;
        }

        break;  /* okay we got one */
    }
    if (s < 0) {
        err(1, "%s", cause);
        /*NOTREACHED*/
    }
    freeaddrinfo(res0);


    /*int pipe = open(path, O_RDONLY, O_NONBLOCK, 0);

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
    */

    return 0;
}

SDL_Thread* InputEvent::demo() {
    return SDL_CreateThread(demoThread, "DemoThread", this);
}

SDL_Thread* InputEvent::fileInput(const char *path) {
    return SDL_CreateThread(fileThread, "FileThread", new fileType(this, path));
}
