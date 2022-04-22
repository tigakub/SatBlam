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
            
            Semaphore sendSem;
            mutex sendMutex;
            MessageQueue sendQueue;
            Semaphore recvSem;
            mutex recvMutex;
            MessageQueue recvQueue;

            MessageFunctor &recvFunctor;

            atomic_bool alive;

            thread *sendThread, *recvThread;

        public:
            UDP(MessageFunctor &iRecvFunctor);
            virtual ~UDP();
            
            void setLocal(const string &iLocalHost, uint16_t iLocalPort);
            void setRemote(const string &iRemoteHost, uint16_t iRemotePort);

            bool start();
            void stop();

            ssize_t send(void *iBuf, size_t iLen);
            ssize_t recv(void *iBuf, size_t iLen);

            void sendLoop();
            void recvLoop();

            void queueForSend(Message *iMsg);
            void queueForRecv(Message *iMsg);

        protected:
            MessagePtr deqSendMsg();
            MessagePtr deqRecvMsg();
            static void sendProc(UDP *iSelf);
            static void recvProc(UDP *iSelf);
    };
}

#endif
