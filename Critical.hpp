#ifndef __CRITICAL_HPP__
#define __CRITICAL_HPP__

#include <mutex>
#include <condition_variable>
#include <chrono>

using namespace std;

namespace sb {
    
    typedef unique_lock<mutex> Grab;

    class Semaphore {
        protected:
            mutex m;
            condition_variable c;
            int count;
            
            int inc() {
                Grab g(m);
                count++;
                return count;
            }
            
            int dec() {
                Grab g(m);
                count--;
                return count;
            }
            
            int get() {
                Grab g(m);
                return count;
            }
            
        public:
            Semaphore(int iCount = 0) : count(iCount) { }
            
            Semaphore &operator++(int) {
                Grab g(m);
                count++;
                c.notify_one();
                return *this;
            }
            
            Semaphore &operator--(int) {
                Grab g(m);
                while(!count)
                    c.wait(g);
                count--;
                return *this;
            }
            
            Semaphore &operator-=(double iDuration) {
                chrono::microseconds t(uint64_t(iDuration * 1000000));
                Grab g(m);
                while(!count)
                    if(c.wait_for(g, t) == cv_status::timeout) {
                        return *this;
                    }
                count--;
                return *this;
            }
    };
}

#endif
