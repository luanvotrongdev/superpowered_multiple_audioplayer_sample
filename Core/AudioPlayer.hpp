#ifndef AudioPlayer_hpp
#define AudioPlayer_hpp

#include <memory>
#include <string>
#include <mutex>

#include "PlayerManagerType.hpp"
#include "AudioSource.hpp"
#include "SuperpoweredAdvancedAudioPlayer.h"

namespace audio {

enum AudioPlayerState {
    AudioPlayerState_Initial = 0,
    AudioPlayerState_Buffering,
    AudioPlayerState_Ready,
    AudioPlayerState_Playing,
    AudioPlayerState_Paused,
    AudioPlayerState_Error
};

class AudioPlayer {
private:
    std::weak_ptr<PlayerManagerType> manager;
    std::shared_ptr<AudioSource> source;
    std::mutex mutex;
    
    AudioPlayerState state;
    AudioPlayerState nextState;
    
    double initialPosition;
    double initialPlaybackRate;
    
    Superpowered::AdvancedAudioPlayer *player;
    
public:
    AudioPlayer(std::weak_ptr<PlayerManagerType> manager);
    virtual ~AudioPlayer();
    
    void setSource(std::shared_ptr<AudioSource> source);
    void play();
    void pause();
    
    void seekPosition(double percent);
    void setPlaybackRate(double rate);
    
    const std::string& getSource() const;
    const AudioPlayerState getState() const;
    const double getPosition() const;
    const double getPlaybackRate() const;
    
    void updateState(const std::shared_ptr<AudioSource> source);
    bool audioProcessing(float *leftOutput, float *rightOutput, unsigned int numberOfFrames, unsigned int samplerate, bool mix, float *outputBuffer);
};

}

#endif /* AudioPlayer_hpp */
