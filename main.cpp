#include "App.hpp"
#include "Message.hpp"
#include "UDP.hpp"
#include "UDPMessenger.hpp"
#include "FrameBlaster.hpp"

#include <iostream>
#include <vector>

using namespace std;

using namespace sb;

class UDPApp : public App {
    protected:
        MessageFunctorT<UDPApp> recvFunctor;
        UDP udp;
        UDPMessenger messenger;
        Semaphore procSem;
        mutex procMutex;
        MessageQueue procQueue;
        thread procThread;
        bool sender;
        uint8_t sendBuffer[1600];

    public:
        UDPApp(const vector<string> &iArgs)
        : App(),
          recvFunctor(*this, &UDPApp::recvMessage),
          udp(),
          messenger(udp, recvFunctor),
          procSem(), procThread(),
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
            procThread = thread(UDPApp::process, this);
            
            cout << "Starting udp" << endl;
            for(int i = 0; i < 5; i++) {
                messenger.queueForRecv(new Message(1600));
            }
            
            messenger.start();
        }

        virtual void mainLoop() {
            if(sender) {
                Message *msg = new Message(sendBuffer, 1600);
                messenger.queueForSend(msg);
            }
        }

        virtual void tearDown() {
            cout << "Stopping udp" << endl;
            messenger.stop();
            if(procThread.joinable()) {
                procSem++;
                procThread.join();
            }
        }
        
        void processLoop() {
            while(!timeToQuit()) {
                procSem--;
                Grab g(procMutex);
                if(procQueue.size()) {
                    MessagePtr ptr(procQueue.front());
                    procQueue.pop_front();
                    cout << "Message received" << endl;
                    messenger.queueForRecv((Message *) ptr);
                }
            }
            cout << "Stopping processing thread" << endl;
        }

        virtual void recvMessage(MessagePtr iMsg) {
            {
                Grab g(procMutex);
                procQueue.push_back(iMsg);
            }
            procSem++;
        }
        
    protected:
        static void process(UDPApp *iSelf) {
            iSelf->processLoop();
        }
};

class FrameBlasterApp : public App, public FrameBlaster {
    protected:
        uint16_t *frame;
        Semaphore throttle;
        
    public:
        FrameBlasterApp(UDP &iUDP, uint16_t *iFrame)
        : FrameBlaster(iUDP, 640 * sizeof(uint16_t), 400), frame(iFrame) { }
        
        virtual void mainLoop() {
            throttle -= 1.0;
            blast((char *) frame);
            cout << "Blast frame " << frameNumber << endl;
        }
};

class FrameCoalescerApp : public App, public FrameCoalescer {
    protected:
        uint16_t *frame;
        
    public:
        FrameCoalescerApp(UDP &iUDP, uint16_t *iFrame)
        : FrameCoalescer(iUDP, 640 * sizeof(uint16_t), 400), frame(iFrame) { }
        
        virtual void mainLoop() {
            coalesce((char *) frame);
        }
        
        virtual void frameStarted() {
            cout << "Frame " << frameNumber << " started" << endl;
        }
        virtual void frameComplete() {
            cout << "Frame " << frameNumber << " complete" << endl;
        }
};

int main(int argc, char **argv) {
    vector<string> args;
    for(int i = 1; i < argc; i++) {
        args.push_back(string(argv[i]));
    }
    
    UDP udp;
    bool sender = false;
    
    string localHost("127.0.0.1");
    uint16_t localPort = 3060;
    string remoteHost("127.0.0.1");
    uint16_t remotePort = 3060;
    
    for(int i = 0; i < args.size(); i++) {
        if(args[i] == "--remote" && (args.size() > i+2)) {
            cout << "Remote: " << args[i+1] << ":" << args[i+2] << endl;
            remoteHost = args[i+1];
            remotePort = stoi(args[i+2]);
            i += 2;
        }
        if(args[i] == "--local" && (args.size() > i+2)) {
            cout << "Local: " << args[i+1] << ":" << args[i+2] << endl;
            localHost = args[i+1];
            localPort = stoi(args[i+2]);
            i += 2;
        }
        if(args[i] == "--blast") {
            cout << "FrameBlaster" << endl;
            sender = true;
        }
    }
    
    if(!sender) {
        cout << "FrameCoalescer" << endl;
    }
    
    udp.setLocal(localHost, localPort);
    udp.setRemote(remoteHost, remotePort);
    
    uint16_t *theFrame = new uint16_t[640 * 400];
    
    if(sender) {
        FrameBlasterApp app(udp, theFrame);
        app.run();
    } else {
        FrameCoalescerApp app(udp, theFrame);
        app.run();
    }
    
    /*
    UDPApp app(args);
    */
    
    delete [] theFrame;
    
    return 0;
}
