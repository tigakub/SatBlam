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

    public:
        UDPApp(const string &iRemoteHost, uint16_t iRemotePort)
        : App(),
          recvFunctor(*this, &UDPApp::recvMessage),
          udp(iRemoteHost, iRemotePort, recvFunctor) { }

        virtual void setUp() {
            cout << "Starting udp" << endl;
            for(int i = 0; i < 5; i++) {
                udp.queueForRecv(new Message(1600));
            }
            udp.start(3060);
        }

        virtual void mainLoop() {
        }

        virtual void tearDown() {
            cout << "Stopping udp" << endl;
            udp.stop();
        }

        virtual void recvMessage(Message &iMsg) {
        }
};

int main(int argc, char **argv) {
    UDPApp app(string(argv[1]), 3060);

    app.run();

    return 0;
}
