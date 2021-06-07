#ifndef LoopHandler_hpp
#define LoopHandler_hpp

#include <functional>
#include <memory>

namespace audio {

    class LoopHandler {
    private:
        void *data;
        
        void start();
        void notify();
        void runAll();
        void addInternal(std::function<void ()> runnable);
        
        LoopHandler();
        
        static std::shared_ptr<LoopHandler> instance;
        
    public:
        static std::shared_ptr<LoopHandler> getInstance();
        static void add(std::function<void ()> runnable);
        
        virtual ~LoopHandler();
    };

}

#endif /* LoopHandler_hpp */
