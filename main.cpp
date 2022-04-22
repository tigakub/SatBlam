#include "App.hpp"
#include "Message.hpp"
#include "UDP.hpp"

#include <iostream>

using namespace std;

using namespace sb;

class UDPApp : public App {
    protected:
        MessageFunctorT<UDPApp> recvFunctor;
        UDP udp;
        Semaphore procSem;
        mutex procMutex;
        MessageQueue procQueue;
        thread *procThread;

    public:
        UDPApp(const string &iRemoteHost, uint16_t iRemotePort)
        : App(),
          recvFunctor(*this, &UDPApp::recvMessage),
          udp(iRemoteHost, iRemotePort, recvFunctor),
          procSem(), procThread(nullptr) { }

        virtual void setUp() {
            cout << "Starting udp" << endl;
            for(int i = 0; i < 5; i++) {
                udp.queueForRecv(new Message(1600));
            }
            procThread = new thread(UDPApp::process, this);
            
            udp.start(3060);
        }

        virtual void mainLoop() {
        }

        virtual void tearDown() {
            cout << "Stopping udp" << endl;
            udp.stop();
        }
        
        void processLoop() {
            while(!timeToQuit()) {
                procSem.wait();
                Grab g(procMutex);
                MessagePtr ptr(procQueue.front());
                cout << "Message received" << endl;
                udp.queueForRecv((Message *) ptr);
            }
        }

        virtual void recvMessage(MessagePtr iMsg) {
            {
                Grab g(procMutex);
                procQueue.push_back(iMsg);
            }
            procSem.signal();
        }
        
    protected:
        static void process(UDPApp *iSelf) {
            iSelf->processLoop();
        }
};

int main(int argc, char **argv) {
    UDPApp app(string(argv[1]), 3060);

    app.run();

    return 0;
}
