#include "UDPMessenger.hpp"

#include <unistd.h> // for close
#include <string.h> // for memset
#include <sys/socket.h>  // socket

using namespace sb;

UDPMessenger::UDPMessenger(UDP &iUDP, MessageFunctor &iRecvFunctor)
: udp(iUDP),
  sendSem(),
  sendMutex(), sendQueue(),
  recvSem(),
  recvMutex(), recvQueue(),
  recvFunctor(iRecvFunctor),
  alive(true),
  sendThread(nullptr), recvThread(nullptr) { }

bool UDPMessenger::start() {
    if(!udp.start()) return false;

    sendThread = new thread(UDPMessenger::sendProc, this);
    recvThread = new thread(UDPMessenger::recvProc, this);

    return true;
}

void UDPMessenger::stop() {
    alive = false;
    if(sendThread) {
        sendSem.signal();
        sendThread->join();
        delete sendThread;
        sendThread = nullptr;
    }
    if(recvThread) {
        recvSem.signal();
        recvThread->join();
        delete recvThread;
        recvThread = nullptr;
    }
    udp.stop();
}

void UDPMessenger::sendLoop() {
    while(alive) {
        sendSem.wait();
        if(sendQueue.size()) {
            MessagePtr currentMsg(deqSendMsg());
            Message &msg = *currentMsg;
            
            bool retry = false;
            do {
                switch(udp.send(msg.buf, msg.length)) {
                    case EWOULDBLOCK:
                        retry = true;
                        break;
                    default:
                        retry = false;
                }
            } while(retry && alive);
        }
    }
}

void UDPMessenger::recvLoop() {
    while(alive) {
        recvSem.wait();
        if(recvQueue.size()) {
            MessagePtr currentMsg(deqRecvMsg());
            Message &msg = *currentMsg;

            bool retry = false;
            do {
                ssize_t byteCount = udp.recv(msg.buf, msg.length);
                switch(byteCount) {
                    case EWOULDBLOCK:
                        retry = true;
                        break;
                    default:
                        if(byteCount > 0) {
                            recvFunctor(currentMsg);
                        } else if(byteCount < 0) {
                            alive = false;
                        } else {
                            retry = true;
                        }
                }
            } while(retry && alive);
        }
    }
}

void UDPMessenger::queueForSend(Message *iMsg) {
    Grab g(sendMutex);
    sendQueue.push_back(MessagePtr(iMsg));
    sendSem.signal();
}

void UDPMessenger::queueForRecv(Message *iMsg) {
    Grab g(recvMutex);
    recvQueue.push_back(MessagePtr(iMsg));
    recvSem.signal();
}

MessagePtr UDPMessenger::deqSendMsg() {
    Grab g(sendMutex);
    MessagePtr ptr(sendQueue.front());
    sendQueue.pop_front();
    return ptr;
}

MessagePtr UDPMessenger::deqRecvMsg() {
    Grab g(recvMutex);
    MessagePtr ptr(recvQueue.front());
    recvQueue.pop_front();
    return ptr;
}

void UDPMessenger::sendProc(UDPMessenger *iSelf) {
    iSelf->sendLoop();
}

void UDPMessenger::recvProc(UDPMessenger *iSelf) {
    iSelf->recvLoop();
}
