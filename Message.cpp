#include "Message.hpp"

#include <sys/socket.h>  // socket

using namespace sb;

Message::Message(uint32_t iLen)
: Object(), buf(nullptr), length(iLen), noDelete(false) {
    buf = new uint8_t[iLen];
}

Message::Message(uint8_t *iBuf, uint32_t iLen)
: Object(), buf(iBuf), length(iLen), noDelete(true) { }

Message::Message(Message &&iToMove)
: Object(), buf(iToMove.buf), length(iToMove.length), noDelete(iToMove.noDelete) {
    iToMove.buf = nullptr;
    iToMove.length = 0;
}

Message::~Message() {
    if(noDelete) return;
    if(buf) delete [] buf;
}

MessagePtr::MessagePtr(Message *iMsg)
: SmartPtrT<Message>(iMsg) { }

MessageQueue::MessageQueue()
: deque<MessagePtr>() { }
