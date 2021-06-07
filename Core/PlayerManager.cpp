#include "PlayerManager.hpp"
#include "Debug.hpp"
#include "SuperpoweredSimple.h"

using namespace audio;

PlayerManager::PlayerManager()
    : sourceMap()
    , self(this)
    , playerList()
    , outputBuffer(NULL)
    , numberOfFrames(0)
    , audioProcessingMutex()
{
    printDebug("[PlayerManager] Created");
}

PlayerManager::~PlayerManager() {
    if (outputBuffer != NULL) {
        delete outputBuffer;
    }
    
    printDebug("[PlayerManager] Deleted");
}

void PlayerManager::notifyStateChanged() {
    LoopHandler::add([this] () {
        std::lock_guard<std::mutex> guard(audioProcessingMutex);
        std::map<std::string, bool> unusedSourceMap;
        
        for (auto it = sourceMap.begin(); it != sourceMap.end(); it++) {
            unusedSourceMap[(*it).first] = true;
        }
        
        for (auto it = playerList.begin(); it != playerList.end(); it++) {
            if (auto player = (*it).lock()) {
                auto sourcePtr = sourceMap.find(player->getSource());
                
                if (sourcePtr != sourceMap.end()) {
                    unusedSourceMap[(*sourcePtr).first] = false;
                    unusedSourceMap[player->getHoldingSource()] = false;
                    
                    player->updateState((*sourcePtr).second);
                }
            }
        }
        
        for (auto it = unusedSourceMap.begin(); it != unusedSourceMap.end(); it++) {
            if ((*it).second) {
                sourceMap.erase((*it).first);
            }
        }
    });
}

void PlayerManager::preloadSource(const std::string &source) {
    LoopHandler::add(std::bind([this] (const std::string &source) {
        if (sourceMap.find(source) == sourceMap.end()) {
            auto audioSource = AudioSource::init(source, self);
            sourceMap[source] = audioSource;
        }
    }, source));
}

void PlayerManager::loadSource(const std::string &source, const std::function<void (std::shared_ptr<AudioSource>)> &callback) {
    LoopHandler::add(std::bind([this] (const std::string &source, const std::function<void (std::shared_ptr<AudioSource>)> &callback) {
        
        auto it = sourceMap.find(source);
        
        if (it == sourceMap.end()) {
            auto audioSource = AudioSource::init(source, self);
            sourceMap[source] = audioSource;
            callback(audioSource);
        } else {
            callback((*it).second);
        }
        
    }, source, callback));
}

void PlayerManager::createPlayer(const std::function<void (std::shared_ptr<AudioPlayer>)> &callback) {
    LoopHandler::add(std::bind([this] (const std::function<void (std::shared_ptr<AudioPlayer>)> &callback) {
        std::lock_guard<std::mutex> guard(audioProcessingMutex);
        
        auto player = std::make_shared<AudioPlayer>(self);
        
        playerList.push_back(player);
        
        callback(player);
        
    }, callback));
}

bool PlayerManager::audioProcessing(float *leftOutput, float *rightOutput, unsigned int numberOfFrames, unsigned int samplerate) {
    std::lock_guard<std::mutex> guard(audioProcessingMutex);
    
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
