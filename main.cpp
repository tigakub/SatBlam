#include "App.hpp"
#include "Message.hpp"
#include "UDP.hpp"

#include <iostream>
#include <vector>

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
        bool sender;

    public:
        UDPApp(const vector<string> &iArgs)
        : App(),
          recvFunctor(*this, &UDPApp::recvMessage),
          udp(recvFunctor),
          procSem(), procThread(nullptr),
          sender(false) {
            string localHost("127.0.0.1");
            uint16_t localPort = 3060;
            string remoteHost("127.0.0.1");
            uint16_t remotePort = 3060;
            for(int i = 0; i < iArgs.size(); i++) {
                if(iArgs[i] == "-r" && (iArgs.size() > i+2)) {
                    cout << "Remote: " << iArgs[i+1] << ":" << iArgs[i+2] << endl;
                    remoteHost = iArgs[i+1];
                    remotePort = stoi(iArgs[i+2]);
                    i += 2;
                }
                if(iArgs[i] == "-b" && (iArgs.size() > i+2)) {
                    cout << "Local: " << iArgs[i+1] << ":" << iArgs[i+2] << endl;
                    localHost = iArgs[i+1];
                    localPort = stoi(iArgs[i+2]);
                    i += 2;
                }
                if(iArgs[i] == "-s") {
                    cout << "Sender" << endl;
                    sender = true;
                }
            }
            udp.setLocal(localHost, localPort);
            udp.setRemote(remoteHost, remotePort);
        }

        virtual void setUp() {
            cout << "Sarting processing thread" << endl;
            procThread = new thread(UDPApp::process, this);
            
            cout << "Starting udp" << endl;
            for(int i = 0; i < 5; i++) {
                udp.queueForRecv(new Message(1600));
            }
            
            udp.start("10.0.1.10", 3060);
        }

        virtual void mainLoop() {
            if(sender) {
                Message *msg = new Message(1600);
                udp.queueForSend(msg);
            }
        }

        virtual void tearDown() {
            cout << "Stopping udp" << endl;
            udp.stop();
            if(procThread) {
                procSem.signal();
                procThread->join();
            }
        }
        
        void processLoop() {
            while(!timeToQuit()) {
                procSem.wait();
                Grab g(procMutex);
                if(procQueue.size()) {
                    MessagePtr ptr(procQueue.front());
                    procQueue.pop_front();
                    cout << "Message received" << endl;
                    udp.queueForRecv((Message *) ptr);
                }
            }
            cout << "Stopping processing thread" << endl;
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
    vector<string> args;
    for(int i = 1; i < argc; i++) {
        args.push_back(string(argv[i]));
    }
    UDPApp app(args);

    app.run();

    return 0;
}
