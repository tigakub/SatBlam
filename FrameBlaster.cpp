#include "FrameBlaster.hpp"

using namespace sb;

void threadProc(FrameProcessor *iTarget, FrameProcessor::RowArg iArg) {
    iTarget->process(iArg);
}

FrameProcessor::FrameProcessor(int iRowByteSize, int iRowCount)
: threadPool(), rowBuffer(), rowByteSize(iRowByteSize + sizeof(RowHeader)) {
    int i = iRowCount;
    while(i--) {
        threadPool.push_back(thread());
        rowBuffer.push_back(new char[iRowByteSize]);
    }
}

FrameProcessor::~FrameProcessor() {
    size_t i = rowBuffer.size();
    while(i--) {
        if(rowBuffer[i]) delete [] rowBuffer[i];
    }
}

void FrameProcessor::start(FrameProcessor::RowArg iArg) {
    threadPool[iArg.rowNumber] = thread(threadProc, this, iArg);
}

FrameBlaster::FrameBlaster(UDP &iUDP, int iRowByteSize, int iRowCount)
: FrameProcessor(iRowByteSize, iRowCount), udp(iUDP), frameNumber(0) {
}

void FrameBlaster::blast(char *iFrame) {
    RowArg arg;
    arg.frameNumber = frameNumber;
    frameStarted();
    for(int i = 0; i < threadPool.size(); i++) {
        arg.rowNumber = i;
        arg.bytePtr = iFrame;
        start(arg);
        iFrame += rowByteSize;
    }
    for(int i = 0; i < rowBuffer.size(); i++) {
        if(threadPool[i].joinable()) {
            threadPool[i].join();
        }
        udp.send(rowBuffer[i], rowByteSize);
    }
    frameComplete();
    frameNumber++;
}

void FrameBlaster::process(RowArg iArg) {
    RowHeader *header = (RowHeader *) rowBuffer[iArg.rowNumber];
    header->rowNumber = iArg.rowNumber;
    header->frameNumber = iArg.frameNumber;
    header->timeStamp = iArg.timeStamp;
    char *bytes = rowBuffer[iArg.rowNumber] + sizeof(header);
    memcpy(bytes, iArg.bytePtr, rowByteSize);
}

FrameCoalescer::FrameCoalescer(UDP &iUDP, int iRowByteSize, int iRowCount)
: FrameProcessor(iRowByteSize, iRowCount), udp(iUDP) {
    recvBuffer = new char[iRowByteSize + sizeof(RowHeader)];
}

FrameCoalescer::~FrameCoalescer()
{
    if(recvBuffer) delete [] recvBuffer;
}

void FrameCoalescer::coalesce(char *oFrame) {
    RowArg arg;
    int recvSize = rowByteSize + sizeof(RowHeader);
    if(udp.recv(recvBuffer, recvSize) == recvSize) {
        RowHeader *header = (RowHeader *) recvBuffer;
        if(header->rowNumber < rowBuffer.size()) {
            arg.rowNumber = header->rowNumber;
            arg.bytePtr = oFrame + header->rowNumber * rowByteSize;
            if(arg.frameNumber > frameNumber) {
                frameNumber = arg.frameNumber;
                rowCount = 0;
                frameStarted();
            }
            char *swap = rowBuffer[header->rowNumber];
            rowBuffer[header->rowNumber] = recvBuffer;
            recvBuffer = swap;
            start(arg);
            rowCount++;
            if(rowCount >= rowBuffer.size()) {
                frameComplete();
            }
        }
    }
}

void FrameCoalescer::process(RowArg iArg) {
    RowHeader *header = (RowHeader *) rowBuffer[iArg.rowNumber];
    header->rowNumber = iArg.rowNumber;
    header->frameNumber = iArg.frameNumber;
    header->timeStamp = iArg.timeStamp;
    char *bytes = rowBuffer[iArg.rowNumber] + sizeof(RowHeader);
    memcpy(iArg.bytePtr, bytes, rowByteSize);
}
