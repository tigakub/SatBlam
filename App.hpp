#ifndef __APP_HPP__
#define __APP_HPP__

namespace sb {

    class App {
        private:
            bool singletonObtained = false;
            
        protected:
            bool echo = false;

        public:
            App();
            virtual ~App();

            void run();
            void quit();

            virtual void setUp();
            virtual void mainLoop();
            virtual void tearDown();

            virtual void pollKey();
            virtual void onKey(char iKey);
    };

}

#endif
