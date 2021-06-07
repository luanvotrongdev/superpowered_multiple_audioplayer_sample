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

// MARK: - AudioSourceImpl

AudioSourceImpl::AudioSourceImpl(const std::string &source, Superpowered::httpRequest *request, std::weak_ptr<PlayerManagerType> manager)
    : AudioSource(source)
    , request(request)
    , state(AudioSourceState_Buffering)
    , data(NULL)
    , manager(manager)
{}

AudioSourceImpl::~AudioSourceImpl() {
    // TODO delete request?
    
    if (data != NULL) {
        // TODO is data a list of char?
        delete [] (char *)data;
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
        // TODO is value a list of char?
        delete [] (char *) value;
        return;
    }
    
    if (data != NULL) {
        // TODO is data a list of char?
        delete [] (char *) data;
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
    auto index = (long) clientData;
    auto it = audioSourceMap.find(index);
    
    if (it == audioSourceMap.end()) {
        printDebug("[audio::requestCallback] index = %ld, not found", index);
        return false;
    }
    
    if (auto audioSource = (*it).second.lock()) {
        switch (response->statusCode) {
            case Superpowered::httpResponse::StatusCode_Success: {
                char *data = new char[response->dataOrFileSizeBytes];
                memcpy(data, response->data, response->dataOrFileSizeBytes);
                
                void *dataDecoded = Superpowered::Decoder::decodeToAudioInMemory(data, response->dataOrFileSizeBytes);
                
                // TODO Is data already released?
                
                audioSource->setData(dataDecoded);
                audioSource->setState(AudioSourceState_Ready);
                
                cleanAudioSource();
                return false;
            }
                
            case Superpowered::httpResponse::StatusCode_Progress: {
                return true;
            }
                
            default: {
                audioSource->setState(AudioSourceState_Error);
                
                cleanAudioSource();
                return false;
            }
        }
    } else {
        printDebug("[audio::requestCallback] index = %ld, audioSource is Deleted", index);
        cleanAudioSource();
        return false;
    }
}
