#ifndef __FRAMEBLASTER_HPP__
#define __FRAMEBLASTER_HPP__

#include "UDP.hpp"

#include <stdio.h>
#include <cstring>

#include <ctime>
#include <thread>
#include <vector>

#include "Critical.hpp"
#include "Thread.hpp"

using namespace std;

namespace sb {
    
    class FrameProcessor {
        protected:
            vector<thread> threadPool;
            vector<char *> rowBuffer;
            int rowByteSize;
            
        public:
            typedef struct RowHeader {
                uint32_t rowNumber;
                uint32_t frameNumber;
                float timeStamp;
            } RowHeader;
            
            typedef struct RowArg {
                uint32_t rowNumber;
                uint32_t frameNumber;
                float timeStamp;
                char *bytePtr;
            } RowArg;
            
            FrameProcessor(int iRowByteSize, int iRowCount);
            virtual ~FrameProcessor();
            
            int rowByteCount() {
                return rowByteSize;
            }
            
            int rowCount() {
                return int(rowBuffer.size());
            }
            
            char *rowPtr(int i) {
                return rowBuffer[i];
            }
            
            virtual void process(RowArg iArg) = 0;
            virtual void frameStarted() { }
            virtual void frameComplete() { }
            
        protected:
            void start(RowArg iArg);
    };
    
    class FrameBlaster : public FrameProcessor {
        protected:
            UDP &udp;
            atomic_uint32_t frameNumber;
            
        public:
            FrameBlaster(UDP &iUDP, int iRowByteSize, int iRowCount);
            
            void blast(char *iFrame);
            
            virtual void process(RowArg iArg);
    };
    
    class FrameCoalescer : public FrameProcessor {
        protected:
            UDP &udp;
            char *recvBuffer;
            uint32_t frameNumber;
            uint32_t rowCount;
            
        public:
            FrameCoalescer(UDP &iUDP, int iRowByteSize, int iRowCount);
            virtual ~FrameCoalescer();
            
            void coalesce(char *oFrame);
            
            virtual void process(RowArg iArg);
    };
}

#endif
