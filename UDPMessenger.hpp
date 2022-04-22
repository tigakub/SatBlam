#ifndef __UDPMESSENGER_HPP__
#define __UDPMESSENGER_HPP__

#include "UDP.hpp"
#include "Critical.hpp"
#include "Message.hpp"

#include <stdio.h>

namespace sb {

    class UDPMessenger {
        protected:
            UDP &udp;
            
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
            UDPMessenger(UDP &iUDP, MessageFunctor &iRecvFunctor);
            
            virtual bool start();
            virtual void stop();

            void sendLoop();
            void recvLoop();

            void queueForSend(Message *iMsg);
            void queueForRecv(Message *iMsg);

        protected:
            MessagePtr deqSendMsg();
            MessagePtr deqRecvMsg();
            static void sendProc(UDPMessenger *iSelf);
            static void recvProc(UDPMessenger *iSelf);
    };

}

#endif
