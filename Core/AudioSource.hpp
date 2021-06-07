#ifndef AudioSource_hpp
#define AudioSource_hpp

#include <string>
#include <memory>
#include "PlayerManagerType.hpp"

namespace audio {

enum AudioSourceState {
    AudioSourceState_Buffering = 0,
    AudioSourceState_Ready,
    AudioSourceState_Error
};

class AudioSource {
protected:
    const std::string source;
    
    AudioSource(const std::string &source);
    
public:
    static std::shared_ptr<AudioSource> init(
        const std::string &source,
        std::weak_ptr<PlayerManagerType> manager
    );
    
    virtual ~AudioSource();
    virtual AudioSourceState getState() = 0;
    virtual void* getData() = 0;
};

}

#endif /* AudioSource_hpp */
