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
    : eventId(SDL_RegisterEvents(1)),
    mutex(SDL_CreateMutex())
{
    assert(eventId >= 0);
    assert(mutex);
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
    if (event.type == SDL_KEYDOWN) {
       SDL_Scancode code = event.key.keysym.scancode;
       //https://wiki.libsdl.org/SDLScancodeLookup
       if (code >= 4 && code <= 39) {
           lock();
           addKey(SDL_GetScancodeName(event.key.keysym.scancode)[0]);
           unlock();
       }
       if (code == SDL_SCANCODE_ESCAPE) {
           radar.clear();
       }
    }
}

void InputEvent::lock() {
   assert(SDL_LockMutex(mutex) == 0);
}

void InputEvent::unlock() {
   assert(SDL_UnlockMutex(mutex) == 0);
}

bool InputEvent::hasKey() {
   lock();
   if (keys.size()) {
       return true;
   }
   else {
      unlock();
      return false;
   }
}

void InputEvent::clear() {
    keys.clear();
}

void InputEvent::addKey(char c) {
    keys.push_back(c);
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
    int sock;
    const char *cause = NULL;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 6; //TCP
    error = getaddrinfo(argv[1], argv[2], &hints, &res0);
    if (error) {
        errx(1, "%s", gai_strerror(error));
        /*NOTREACHED*/
    }
    sock = -1;
    for (res = res0; res; res = res->ai_next) {
        sock = socket(res->ai_family, res->ai_socktype,
                   res->ai_protocol);
        if (sock < 0) {
            cause = "socket";
            continue;
        }

        if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
            cause = "connect";
            close(sock);
            sock = -1;
            continue;
        }

        break;  /* okay we got one */
    }
    if (sock < 0) {
        err(1, "%s", cause);
        /*NOTREACHED*/
    }
    freeaddrinfo(res0);

    std::string str;
    bool lastValid = false;
    unsigned nums = 0;

    while (true) {
        char input[4096] = "";

        fd_set fds;
        timeval tv;
        //Wait for input
        FD_ZERO(&fds);
        FD_SET(sock, &fds);

        //Set waiting time of 10us
        tv.tv_sec = 0;
        tv.tv_usec = 10;

        int s = select(sock + 1, &fds, NULL, NULL, &tv);
        if (s < 0) {
            perror("select");
            return 0;
        }
        if (s == 0) {
            if (ie->hasKey()) {
                //std::cout << "SEND: " << ie->keys << std::endl;
                ie->keys.push_back('\n');
                s = write(sock, ie->keys.c_str(), ie->keys.length());
                if (s < 0) {
                    perror("write");
                    return 0;
                }
                ie->clear();
                ie->unlock();
            }
        }

        if (FD_ISSET(sock, &fds)) {
            int r = read(sock, input, 4096);
            if (r > 0) {
                for (unsigned i = 0; i < r; ++i) {
                    char c = input[i];
                    if ( ('0' <= c && c <= '9') || c == '.') {
                        str.push_back(c);
                        lastValid = true;
                    }
                    if (lastValid && (c == ' ' || c == '\r' || c == '\n')) {
                        str.push_back(' ');
                        lastValid = false;
                        nums ++;
                    }
                }

                while (nums >= 2) {
                    int angle;
                    float distance;

                    std::stringstream ss(str);
                    ss >> angle >> distance;
                    if (ss.good()) {
                        //std::cout << "Input: " << angle << " " << distance << std::endl;
                        ie->pushEvent(Input(angle, distance));
                    }
                    std::string::size_type first = str.find(' ') + 1;
                    str = str.substr(str.find(' ', first) + 1);
                    nums -= 2;
                }
            }
            if (r < 0) {
                perror("read");
                return 0;
            }
            if (r == 0) {
                exit(0);
            }
        }
    }
    close(sock);

    return 0;
}

SDL_Thread* InputEvent::demo() {
    return SDL_CreateThread(demoThread, "DemoThread", this);
}

SDL_Thread* InputEvent::fileInput(const char *path) {
    return SDL_CreateThread(fileThread, "FileThread", new fileType(this, path));
}

SDL_Thread* InputEvent::socketInput(const char **argv) {
   return SDL_CreateThread(socketThread, "SocketThread", new socketType(this, argv));
}
