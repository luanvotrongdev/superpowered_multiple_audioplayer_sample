#ifndef PlayerManager_hpp
#define PlayerManager_hpp

#include <memory>
#include <map>
#include <mutex>
#include <vector>

#include "PlayerManagerType.hpp"
#include "AudioSource.hpp"
#include "AudioPlayer.hpp"
#include "LoopHandler.hpp"

namespace audio {

class PlayerManager : public PlayerManagerType {
private:
    LoopHandler *looper;
    std::shared_ptr<PlayerManagerType> self;
    
    std::map<std::string, std::shared_ptr<AudioSource>> sourceMap;
    std::mutex sourceMutex;
    
    std::vector<std::weak_ptr<AudioPlayer>> playerList;
    std::mutex playerMutex;
    
    float *outputBuffer;
    int numberOfFrames;
    
public:
    PlayerManager();
    virtual ~PlayerManager();
    
    void notifyStateChanged();
    
    void preloadSource(const std::string &source);
    std::shared_ptr<AudioSource> loadSource(const std::string &source);
    
    std::shared_ptr<AudioPlayer> createPlayer();
    
    bool audioProcessing(float *leftOutput, float *rightOutput, unsigned int numberOfFrames, unsigned int samplerate);
};

}

#endif /* PlayerManager_hpp */
