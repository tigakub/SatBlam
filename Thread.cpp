#include "Thread.hpp"

using namespace sb;

void gThreadProc(Thread *iThread) {
    iThread->callback();
}

Thread::Thread()
: t() { }

Thread::~Thread() {
    stop();
}

void Thread::start() {
    t = thread(gThreadProc, this);
}

void Thread::stop() {
    join();
}

void Thread::join() {
    if(t.joinable()) t.join();
}

void Thread::callback() {
}
