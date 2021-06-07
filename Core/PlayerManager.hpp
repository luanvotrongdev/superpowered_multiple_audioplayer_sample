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
    std::map<std::string, std::shared_ptr<AudioSource>> sourceMap;
    std::vector<std::weak_ptr<AudioPlayer>> playerList;
    std::mutex audioProcessingMutex;
    
    float *outputBuffer;
    int numberOfFrames;
    
public:
    PlayerManager();
    virtual ~PlayerManager();
    
    void notifyStateChanged();
    
    void preloadSource(const std::string &source);
    void loadSource(const std::string &source, const std::function<void (std::shared_ptr<AudioSource>)> &callback);
    
    void createPlayer(const std::function<void (std::shared_ptr<AudioPlayer>)> &callback);
    
    bool audioProcessing(float *leftOutput, float *rightOutput, unsigned int numberOfFrames, unsigned int samplerate);
};

}

#endif /* PlayerManager_hpp */
