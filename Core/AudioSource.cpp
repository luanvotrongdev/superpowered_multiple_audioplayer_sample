#include <map>
#include <mutex>

#include "AudioSource.hpp"
#include "Debug.hpp"

#import "SuperpoweredHTTP.h"
#import "SuperpoweredDecoder.h"

using namespace audio;

namespace audio {

class AudioSourceImpl : public AudioSource {
private:
    AudioSourceState state;
    void *data;
    Superpowered::httpRequest *request;
    std::weak_ptr<PlayerManagerType> manager;
    
public:
    AudioSourceImpl(const std::string &source, Superpowered::httpRequest *request, std::weak_ptr<PlayerManagerType> manager);
    virtual ~AudioSourceImpl();
    
    AudioSourceState getState();
    void* getData();
    
    void setState(AudioSourceState state);
    void setData(void *value);
};

long audioSourceCount = 0;
std::mutex audioSourceMutex;
std::map<long, std::weak_ptr<AudioSourceImpl>> audioSourceMap;

void cleanAudioSource();
bool requestCallback(void *clientData, Superpowered::httpRequest *request, Superpowered::httpResponse *response);

}

// MARK: - AudioSource

std::shared_ptr<AudioSource> AudioSource::init(const std::string &source, std::weak_ptr<PlayerManagerType> manager) {
    std::lock_guard<std::mutex> guard(audioSourceMutex);
    
    auto request = new Superpowered::httpRequest(source.c_str());
    auto value = std::shared_ptr<AudioSourceImpl>(new AudioSourceImpl(source, request, manager));
    auto index = audioSourceCount++;
    
    audioSourceMap[index] = std::weak_ptr<AudioSourceImpl>(value);
    request->sendAsync(audio::requestCallback, (void *) index, NULL, false, NULL);
    
    return value;
}

AudioSource::AudioSource(const std::string &source)
    : source(source)
{
    printDebug("[AudioSource] init with source = %s", source.c_str());
}

AudioSource::~AudioSource()
{
    printDebug("[AudioSource] deleted, source = %s", source.c_str());
}

const std::string &AudioSource::getSource() const {
    return source;
}

// MARK: - AudioSourceImpl

AudioSourceImpl::AudioSourceImpl(const std::string &source, Superpowered::httpRequest *request, std::weak_ptr<PlayerManagerType> manager)
    : AudioSource(source)
    , request(request)
    , state(AudioSourceState_Buffering)
    , data(NULL)
    , manager(manager)
{}

AudioSourceImpl::~AudioSourceImpl() {
    // TODO: check delete request?
    
    if (data != NULL) {
        Superpowered::AudioInMemory::release(data);
    }
    
    printDebug("[AudioSourceImpl] deleted, source = %s", source.c_str());
}

AudioSourceState AudioSourceImpl::getState() {
    return state;
}

void AudioSourceImpl::setState(AudioSourceState state) {
    if (auto m = manager.lock()) {
        printDebug("[AudioSourceImpl] setState: %d (1 = READY, 2 = ERROR), source = %s", state, source.c_str());
        this->state = state;
        m->notifyStateChanged();
    }
}

void AudioSourceImpl::setData(void *value) {
    if (manager.expired()) {
        Superpowered::AudioInMemory::release(data);
        return;
    }
    
    if (data != NULL) {
        Superpowered::AudioInMemory::release(data);
    }
    
    data = value;
}

void* AudioSourceImpl::getData() {
    return data;
}

// MARK: - Static Functions

void audio::cleanAudioSource() {
    if (audioSourceMap.size() % 5 == 0) {
        std::lock_guard<std::mutex> guard(audioSourceMutex);
        
        long expiredKeys[audioSourceMap.size()];
        int index = 0;
        
        for (auto it = audioSourceMap.begin(); it != audioSourceMap.end(); it++) {
            if ((*it).second.expired()) {
                expiredKeys[index] = (*it).first;
                index++;
            }
        }
        
        for (int i = 0; i < index; i++) {
            audioSourceMap.erase(audioSourceMap.find(expiredKeys[i]));
        }
    }
}

bool audio::requestCallback(void *clientData, Superpowered::httpRequest *request, Superpowered::httpResponse *response) {
    switch (response->statusCode) {
        case Superpowered::httpResponse::StatusCode_Success: {
            auto index = (long) clientData;
            auto it = audioSourceMap.find(index);
            
            if (it == audioSourceMap.end()) {
                printDebug("[audio::requestCallback] index = %ld, not found", index);
                return false;
            }
            
            if (auto audioSource = (*it).second.lock()) {
                char *dataResponse = response->data;
                unsigned int dataSize = response->dataOrFileSizeBytes;
                
                printDebug("[audio::requestCallback] audio::requestCallback = %ld, dataOrFileSizeBytes = %u, source = %s", (long) dataResponse, dataSize, audioSource->getSource().c_str());
                
                char *data = new char[dataSize];
                memcpy(data, dataResponse, dataSize);
                
                void *dataDecoded = Superpowered::Decoder::decodeToAudioInMemory(data, dataSize);
                
                // TODO: check delete data?
                
                audioSource->setData(dataDecoded);
                audioSource->setState(AudioSourceState_Ready);
                
                cleanAudioSource();
            }
            return false;
        }
            
        case Superpowered::httpResponse::StatusCode_Progress: {
            return true;
        }
            
        default: {
            auto index = (long) clientData;
            auto it = audioSourceMap.find(index);
            
            if (it == audioSourceMap.end()) {
                printDebug("[audio::requestCallback] index = %ld, not found", index);
                return false;
            }
            
            if (auto audioSource = (*it).second.lock()) {
                audioSource->setState(AudioSourceState_Error);
                cleanAudioSource();
            }
            return false;
        }
    }
}
