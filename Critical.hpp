#ifndef __CRITICAL_HPP__
#define __CRITICAL_HPP__

#include <mutex>
#include <condition_variable>

using namespace std;

namespace sb {
    
    typedef unique_lock<mutex> Grab;

    class Semaphore {
        protected:
            mutex m;
            condition_variable c;
            int count;
        
        public:
            Semaphore(int iCount = 0) : count(iCount) { }
            void signal() { Grab g(m); count++; c.notify_one(); }
            void wait() { Grab g(m); while(!count) c.wait(g); count--; }
    };
}

#endif