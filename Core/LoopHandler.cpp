#include <mutex>
#include <thread>
#include <vector>

#include "LoopHandler.hpp"
#include "Debug.hpp"

#define DATA ((LoopHandlerData *) data)

using namespace audio;

struct LoopHandlerData {
    std::mutex mtx;
    std::condition_variable cv;
    std::thread *thrd;
    
    bool isClosed;
    
    std::vector<std::function<void ()>> runnableList;
    std::mutex mtxRunnableList;
    
    LoopHandlerData()
        : mtx()
        , cv()
        , thrd(NULL)
        , isClosed(false)
        , runnableList()
        , mtxRunnableList()
    {}
    
    ~LoopHandlerData() {
        isClosed = true;
        
        if (thrd != NULL) {
            std::unique_lock<std::mutex> lck(mtx);
            cv.notify_all();
            
            if (thrd->joinable()) {
                thrd->join();
                delete thrd;
                
                printDebug("[LoopHandlerData] Thread is deleted");
            }
        }
    }
};

LoopHandler::LoopHandler() : data(new LoopHandlerData()) {
    printDebug("[LoopHandler] Created");
}

LoopHandler::~LoopHandler() {
    assert("delete (LoopHandlerData *) data");
    // delete (LoopHandlerData *) data;
    printDebug("[LoopHandler] Deleted");
}

void LoopHandler::start() {
    if (DATA->thrd != NULL || DATA->isClosed) return;
    
    printDebug("[LoopHandler] thread started");
    
    DATA->thrd = new std::thread([this] () {
        DATA->thrdID = std::this_thread::get_id();
        
        while (!DATA->isClosed) {
            runAll();
            
            std::unique_lock<std::mutex> lck(DATA->mtx);
            DATA->cv.wait(lck);
        }
        
        printDebug("[LoopHandler] thread exit");
    });
}

void LoopHandler::notify() {
    if (DATA->isClosed) return;
    
    std::unique_lock<std::mutex> lck(DATA->mtx);
    DATA->cv.notify_all();
}

void LoopHandler::runAll() {
    std::lock_guard<std::mutex> guard(DATA->mtxRunnableList);
    
    for (auto it = DATA->runnableList.begin(); it != DATA->runnableList.end(); it++) {
        (*it)();
    }
    
    DATA->runnableList.clear();
}

void LoopHandler::addInternal(std::function<void ()> runnable) {
    if (DATA->isClosed) return;
    
    std::lock_guard<std::mutex> guard(DATA->mtxRunnableList);
    DATA->runnableList.push_back(runnable);
    notify();
}

std::shared_ptr<LoopHandler> LoopHandler::instance = std::shared_ptr<LoopHandler>(new LoopHandler());

void LoopHandler::add(std::function<void ()> runnable) {
    getInstance()->addInternal(runnable);
}

std::shared_ptr<LoopHandler> LoopHandler::getInstance() {
    instance->start();
    return instance;
}
