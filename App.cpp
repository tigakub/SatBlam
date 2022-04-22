#include "App.hpp"

#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <termios.h>
#include <signal.h>
#include <atomic>
#include <sys/poll.h>

using namespace std;

namespace sb {

    struct termios gStdTerm, gRawTerm;

    void gSaveTerm() {
        tcgetattr(0, &gStdTerm);
    }

    void gSetTermRaw(bool echo = true) {
        tcgetattr(0, &gRawTerm);
        gRawTerm.c_lflag &= ~(ICANON|(echo?0:ECHO));
        tcsetattr(0, TCSANOW, &gRawTerm);
    }

    void gResetTerm() {
        tcsetattr(0, TCSANOW, &gStdTerm);
    }

    struct sigaction gOldsa, gNewsa;
    atomic_bool gSingleton(false);
    atomic_bool gQuit(false);
    void gSigIntHandler(int iSig) {
        gQuit = true;
    }

    void gInterceptSigInt() {
        gNewsa.sa_handler = gSigIntHandler;
        sigemptyset(&gNewsa.sa_mask);
        gNewsa.sa_flags = 0;
        sigaction(SIGINT, NULL, &gOldsa);
        sigaction(SIGINT, &gNewsa, NULL);
    }

    void gRestoreSigInt() {
        sigaction(SIGINT, &gOldsa, NULL);
    }
    
    App::App() {
        if(!gSingleton.exchange(true)) {
            singletonObtained = true;
            gInterceptSigInt();
            gSaveTerm();
        }
    }

    App::~App() {
        if(singletonObtained) {
            gResetTerm();
            gRestoreSigInt();
            gSingleton = false;
        }
    }

    void App::run() {
        setUp();
        gSetTermRaw(echo);
        while(!gQuit) {
            mainLoop();
            pollKey();
        }
        tearDown();
    }

    void App::quit() {
        gQuit = true;
    }
    
    bool App::timeToQuit() {
        return gQuit;
    }

    void App::setUp() {
        cout << "Commencing" << endl;
    }

    void App::mainLoop() {
    }

    void App::tearDown() {
        cout << "Terminating" << endl;
    }

    void App::pollKey() {
        struct pollfd fds;
        int ret = 0;
        fds.fd = STDIN_FILENO;
        fds.events = POLL_IN;
        ret = poll(&fds, 1, 0);
        if(ret > 0) {
            onKey(getc(stdin));
        }
    }

    void App::onKey(char iKey) {
        switch(iKey) {
            case 27:
            case 'q':
            case 'Q':
                quit();
            default:
                break;
        }
    }
}
