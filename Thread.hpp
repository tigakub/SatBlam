#ifndef __THREAD_HPP__
#define __THREAD_HPP__

#include <stdio.h>
#include <thread>

#include "Critical.hpp"

using namespace std;

namespace sb {
    
    class Thread {
        protected:
            thread t;
            
        public:
            Thread();
            virtual ~Thread();
            
            void start();
            void stop();
            
            void join();
            
            virtual void callback();
    };
    
    template <class T, class A>
    class ThreadT : Thread {
        protected:
            T &target;
            void (T::*method)(A);
            A arg;
            
        public:
            ThreadT(T &iTarget, void (T::*iMethod)(A))
            : Thread(), target(iTarget), method(iMethod) { }
            
            void setArg(const A &iArg) {
                arg = iArg;
            }
            
            virtual void callback() {
                (target.*method)(arg);
            }
    };

}


#endif
