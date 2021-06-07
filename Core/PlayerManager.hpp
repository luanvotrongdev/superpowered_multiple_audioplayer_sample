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
    std::shared_ptr<PlayerManagerType> self;
    std::map<std::string, std::weak_ptr<AudioSource>> sourceMap;
    std::vector<std::weak_ptr<AudioPlayer>> playerList;
    
    std::mutex sourceMapMutex;
    std::mutex audioProcessingMutex;
    
    float *outputBuffer;
    int numberOfFrames;
    
public:
    PlayerManager();
    
    virtual ~PlayerManager();
    
    void notifyStateChanged();
    
    std::shared_ptr<AudioSource> loadSource(const std::string &source);
    std::shared_ptr<AudioPlayer> createPlayer();
    
    bool audioProcessing(float *leftOutput, float *rightOutput, unsigned int numberOfFrames, unsigned int samplerate);
};

}

#endif /* PlayerManager_hpp */
