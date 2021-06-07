#ifndef LoopHandler_hpp
#define LoopHandler_hpp

#include <functional>

namespace audio {

    class LoopHandler {
    private:
        void *data;
        
        void notify();
        void runAll();
        
    public:
        LoopHandler();
        virtual ~LoopHandler();
        
        void start();
        void add(std::function<void ()> runnable);
    };

}

#endif /* LoopHandler_hpp */
