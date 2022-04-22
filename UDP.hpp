#ifndef __UDP_HPP__
#define __UDP_HPP__

#include <string>
#include <arpa/inet.h>   // struct sockaddr_in
#include "Message.hpp"
#include "Critical.hpp"
#include <deque>
#include <atomic>
#include <thread>

using namespace std;

namespace sb {

    class UDP {
        protected:
            int sock;
            int sockFlags;
            struct sockaddr_in local;
            struct sockaddr_in remote;
            
        public:
            UDP();
            virtual ~UDP();
            
            void setLocal(const string &iLocalHost, uint16_t iLocalPort);
            void setRemote(const string &iRemoteHost, uint16_t iRemotePort);

            virtual bool start();
            virtual void stop();

            ssize_t send(void *iBuf, size_t iLen);
            ssize_t recv(void *iBuf, size_t iLen);
    };
    
}

#endif
