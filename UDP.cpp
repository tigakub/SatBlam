#include "UDP.hpp"

#include <unistd.h> // for close
#include <string.h> // for memset
#include <sys/socket.h>  // socket

using namespace sb;

UDP::UDP()
: sock(0),
  // sockFlags(0),
  sockFlags(MSG_DONTWAIT) { }

UDP::~UDP() {
    stop();
}

void UDP::setLocal(const string &iLocalHost, uint16_t iLocalPort) {
    memset((char *) &remote, 0, sizeof(remote));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = inet_addr(iLocalHost.c_str());
    local.sin_port = htons(iLocalPort);
}

void UDP::setRemote(const string &iRemoteHost, uint16_t iRemotePort) {
    memset((char *) &remote, 0, sizeof(remote));
    remote.sin_family = AF_INET;
    remote.sin_addr.s_addr = inet_addr(iRemoteHost.c_str());
    remote.sin_port = htons(iRemotePort);
}

bool UDP::start() {
    if(sock) return false;
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if(sock < 0) return false;

    struct sockaddr *castLocal = (struct sockaddr *) &local;

    if(::bind(sock, castLocal, sizeof(local)) < 0) {
        close(sock);
        sock = 0;
        return false;
    }

    return true;
}

void UDP::stop() {
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
