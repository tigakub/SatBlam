#ifndef __SMARTPTR_HPP__
#define __SMARTPTR_HPP__

#include <stdint.h>

using namespace std;

namespace sb {

    class Object {
        friend class SmartPtr;

        protected:
            uint32_t refCount;

        public:
            Object() : refCount(0) { }
            virtual ~Object() { }

            Object *inc() {
                refCount++;
                return this;
            }

            Object *dec() {
                if(refCount) refCount--;
                if(refCount) return this;
                delete this;
                return nullptr;
            }
    };

    class SmartPtr {
        protected:
            Object *obj;

        public:
            SmartPtr(Object *iPtr = nullptr)
            : obj(iPtr) {
                obj && obj->inc();
            }
            SmartPtr(const SmartPtr &iPtr)
            : obj(iPtr.obj) {
                obj && obj->inc();
            }
            SmartPtr(SmartPtr &&iPtr)
            : obj(iPtr.obj) {
                iPtr.obj = nullptr;
            }

            virtual ~SmartPtr() {
                obj && obj->dec();
            }
    };

    template <class T>
    class SmartPtrT : public SmartPtr {
        public:
            SmartPtrT(T *iPtr = nullptr)
            : SmartPtr(iPtr) { }
            SmartPtrT(const SmartPtrT<T> &iPtr)
            : SmartPtr(iPtr) { }
            SmartPtrT(SmartPtrT<T> &&iPtr)
            : SmartPtr(move(iPtr)) { }

            virtual SmartPtrT<T> &operator=(const SmartPtrT<T> &iOther) {
                obj && obj->dec();
                obj = iOther.obj;
                obj && obj->inc();
                return *this;
            }

            virtual SmartPtrT<T> &operator=(SmartPtrT<T> &&iOther) {
                obj && obj->dec();
                obj = iOther.obj;
                iOther.obj = nullptr;
                return *this;
            }

            virtual T& operator*() {
                return (T&) *obj;
            }

            virtual const T& operator*() const {
                return (const T&) *obj;
            }

            virtual T* operator->() {
                return (T*) obj;
            }

            virtual const T* operator->() const {
                return (const T*) obj;
            }
            
            virtual operator T* () {
                return (T *) obj;
            }
            
            virtual operator const T* () const {
                return (const T*) obj;
            }
    };

}

#endif
