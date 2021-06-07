#include "PlayerManager.hpp"
#include "Debug.hpp"
#include "SuperpoweredSimple.h"

using namespace audio;

PlayerManager::PlayerManager()
    : looper(new LoopHandler())
    , sourceMap()
    , sourceMutex()
    , self(this)
    , playerList()
    , playerMutex()
    , outputBuffer(NULL)
    , numberOfFrames(0)
{
    looper->start();
    
    printDebug("[PlayerManager] Created");
}

PlayerManager::~PlayerManager() {
    if (outputBuffer != NULL) {
        delete outputBuffer;
    }
    
    delete looper; // TODO Can't Delete!!!!
    
    printDebug("[PlayerManager] Deleted");
}

void PlayerManager::notifyStateChanged() {
    looper->add([this] () {
        std::lock_guard<std::mutex> guard(playerMutex);
        
        for (auto it = playerList.begin(); it != playerList.end(); it++) {
            if (auto player = (*it).lock()) {
                if (auto source = loadSource(player->getSource())) {
                    player->updateState(source);
                }
            }
        }
    });
}

void PlayerManager::preloadSource(const std::string &source) {
    std::lock_guard<std::mutex> guard(sourceMutex);
    
    if (sourceMap.find(source) == sourceMap.end()) {
        auto audioSource = AudioSource::init(source, self);
        sourceMap[source] = audioSource;
    }
}

std::shared_ptr<AudioSource> PlayerManager::loadSource(const std::string &source) {
    std::lock_guard<std::mutex> guard(sourceMutex);
    
    auto it = sourceMap.find(source);
    
    if (it == sourceMap.end()) {
        auto audioSource = AudioSource::init(source, self);
        sourceMap[source] = audioSource;
        return audioSource;
    } else {
        return (*it).second;
    }
}

std::shared_ptr<AudioPlayer> PlayerManager::createPlayer() {
    std::lock_guard<std::mutex> guard(playerMutex);
    
    auto player = std::make_shared<AudioPlayer>(self);
    
    playerList.push_back(player);
    
    return player;
}

bool PlayerManager::audioProcessing(float *leftOutput, float *rightOutput, unsigned int numberOfFrames, unsigned int samplerate) {
    std::lock_guard<std::mutex> guard(playerMutex);
    
    bool hasSound = false;
    bool mix = false;
    
    for (auto it = playerList.begin(); it != playerList.end(); it++) {
        if (auto player = (*it).lock()) {
            if (this->numberOfFrames != numberOfFrames || outputBuffer == NULL) {
                if (outputBuffer != NULL) {
                    delete outputBuffer;
                }
                
                outputBuffer = new float[numberOfFrames];
                this->numberOfFrames = numberOfFrames;
            }
            
            bool playerHasSound = player->audioProcessing(leftOutput, rightOutput, numberOfFrames, samplerate, mix, outputBuffer);
            
            hasSound |= playerHasSound;
            
            if (playerHasSound) {
                mix = true;
            }
        }
    }
    
    if (hasSound) {
        Superpowered::DeInterleave(outputBuffer, leftOutput, rightOutput, numberOfFrames);
    }
    
    return hasSound;
}
