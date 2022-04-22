#include "UDP.hpp"

#include <unistd.h> // for close
#include <string.h> // for memset
#include <sys/socket.h>  // socket

using namespace sb;

UDP::UDP(const string &iRemoteHost, uint16_t iRemotePort, MessageFunctor &iRecvFunctor)
: sock(0), sockFlags(MSG_DONTWAIT),
  sendSem(),
  sendMutex(), sendQueue(),
  recvSem(),
  recvMutex(), recvQueue(),
  recvFunctor(iRecvFunctor),
  alive(true),
  sendThread(nullptr), recvThread(nullptr) {
    memset((char *) &remote, 0, sizeof(remote));
    remote.sin_family = AF_INET;
    remote.sin_addr.s_addr = inet_addr(iRemoteHost.c_str());
    remote.sin_port = htons(iRemotePort);
}

UDP::~UDP() {
    stop();
}

bool UDP::start(uint16_t iPort) {
    if(sock) return false;
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if(sock < 0) return false;

    struct sockaddr_in local;
    memset((char *) &local, 0, sizeof(local));
    remote.sin_family = AF_INET;
    remote.sin_addr.s_addr = htonl(INADDR_ANY);
    remote.sin_port = htons(iPort);
    struct sockaddr *castLocal = (struct sockaddr *) &local;

    if(::bind(sock, castLocal, sizeof(local)) < 0) {
        close(sock);
        sock = 0;
        return false;
    }

    sendThread = new thread(UDP::sendProc, this);
    recvThread = new thread(UDP::recvProc, this);

    return true;
}

void UDP::stop() {
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
    if(sock) close(sock);
    sock = 0;
}

ssize_t UDP::send(void *iBuf, size_t iLen) {
    struct sockaddr *castRemote = (struct sockaddr *) &remote;
    socklen_t addrLen = sizeof(remote);
    return sendto(sock, iBuf, iLen, sockFlags, castRemote, addrLen) >= 0;
}

ssize_t UDP::recv(void *iBuf, size_t iLen) {
    struct sockaddr *castRemote = (struct sockaddr *) &remote;
    socklen_t addrLen = sizeof(remote);
    return recvfrom(sock, iBuf, iLen, sockFlags, castRemote, &addrLen) >= 0;
}

void UDP::sendLoop() {
    while(alive) {
        sendSem.wait();
        if(sendQueue.size()) {
            MessagePtr currentMsg(move(deqSendMsg()));
            Message &msg = *currentMsg;
            
            bool retry = false;
            do {
                switch(send(msg.buf, msg.length)) {
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

void UDP::recvLoop() {
    while(alive) {
        recvSem.wait();
        if(recvQueue.size()) {
            MessagePtr currentMsg(deqRecvMsg());
            Message &msg = *currentMsg;

            bool retry = false;
            do {
                ssize_t byteCount = recv(msg.buf, msg.length);
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

void UDP::queueForSend(Message *iMsg) {
    Grab g(sendMutex);
    sendQueue.push_back(MessagePtr(iMsg));
    sendSem.signal();
}

void UDP::queueForRecv(Message *iMsg) {
    Grab g(recvMutex);
    recvQueue.push_back(MessagePtr(iMsg));
    recvSem.signal();
}

MessagePtr UDP::deqSendMsg() {
    Grab g(sendMutex);
    MessagePtr ptr(move(sendQueue.front()));
    sendQueue.pop_front();
    return ptr;
}

MessagePtr UDP::deqRecvMsg() {
    Grab g(recvMutex);
    MessagePtr ptr(move(recvQueue.front()));
    recvQueue.pop_front();
    return ptr;
}

void UDP::sendProc(UDP *iSelf) {
    iSelf->sendLoop();
}

void UDP::recvProc(UDP *iSelf) {
    iSelf->recvLoop();
}
