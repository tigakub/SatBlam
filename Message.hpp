#ifndef __MESSAGE_HPP__
#define __MESSAGE_HPP__

#include "SmartPtr.hpp"

#include <deque>

using namespace std;

namespace sb {

    class Message : public Object {
        public:
            uint8_t *buf;
            uint32_t length;
            bool noDelete;

            Message(uint32_t iLen);
            Message(uint8_t *iBuf, uint32_t iLen);
            Message(Message &&iToMove);

            virtual ~Message();
    };

    class MessagePtr : public SmartPtrT<Message> {
        public:
            MessagePtr(Message *iMsg = nullptr);
    };

    class MessageQueue : public deque<MessagePtr> {
        public:
            MessageQueue();
    };

    class MessageFunctor {
        public:
            virtual void operator()(MessagePtr iMsg) = 0;
    };

    template <class T>
    class MessageFunctorT : public MessageFunctor {
        protected:
            T &object;
            void (T::*method)(MessagePtr iPtr);

        public:
            MessageFunctorT(T &iObject, void (T::*iMethod)(MessagePtr))
            : object(iObject), method(iMethod) { }

            virtual void operator()(MessagePtr iMsg) {
                (object.*method)(iMsg);
            }

    };

}

#endif
