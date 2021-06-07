#include "AudioPlayer.hpp"
#include "Debug.hpp"

using namespace audio;

AudioPlayer::AudioPlayer(std::weak_ptr<PlayerManagerType> manager)
    : manager(manager)
    , source("")
    , state(AudioPlayerState_Initial)
    , nextState(AudioPlayerState_Initial)
    , mutex()
    , player(NULL)
    , initialPosition(0)
    , initialPlaybackRate(1)
{
    printDebug("[AudioPlayer] Created");
}

AudioPlayer::~AudioPlayer() {
    if (this->player) {
        delete this->player;
    }
    
    printDebug("[AudioPlayer] Deleted");
}

void AudioPlayer::loadSource(const std::string &source) {
    if (this->source == source) return;
    
    if (auto m = manager.lock()) {
        std::lock_guard<std::mutex> guard(mutex);
        
        printDebug("[AudioPlayer] loadSource: %s", source.c_str());
        
        if (this->player) {
            delete this->player;
            this->player = NULL;
        }
        
        this->source = source;
        this->state = AudioPlayerState_Initial;
        
        m->preloadSource(source);
        m->notifyStateChanged();
    }
}

void AudioPlayer::play() {
    if (nextState == AudioPlayerState_Playing) return;
    
    if (auto m = manager.lock()) {
        printDebug("[AudioPlayer] play");
        
        nextState = AudioPlayerState_Playing;
        m->notifyStateChanged();
    }
}

void AudioPlayer::pause() {
    if (nextState == AudioPlayerState_Paused) return;
    
    if (auto m = manager.lock()) {
        printDebug("[AudioPlayer] pause");
        
        nextState = AudioPlayerState_Paused;
        m->notifyStateChanged();
    }
}

void AudioPlayer::seekPosition(double percent) {
    initialPosition = percent;
    
    if (player != NULL) {
        player->seek(percent);
    }
}

void AudioPlayer::setPlaybackRate(double rate) {
    initialPlaybackRate = rate;
    
    if (player != NULL) {
        player->playbackRate = rate;
    }
}

const std::string& AudioPlayer::getSource() const {
    return source;
}

const AudioPlayerState AudioPlayer::getState() const {
    return state;
}

const double AudioPlayer::getPosition() const {
    if (player != NULL) {
        return player->getDisplayPositionPercent();
    } else {
        return initialPosition;
    }
}

const double AudioPlayer::getPlaybackRate() const {
    if (player != NULL) {
        return player->playbackRate;
    } else {
        return initialPlaybackRate;
    }
}

void AudioPlayer::updateState(const std::shared_ptr<AudioSource> source) {
    std::lock_guard<std::mutex> guard(mutex);
    
    if (state == AudioPlayerState_Initial || state == AudioPlayerState_Buffering || state == AudioPlayerState_Error) {
        auto sourceState = source->getState();
        
        switch (sourceState) {
            case AudioSourceState_Buffering: {
                printDebug("[AudioPlayer] state = AudioPlayerState_Buffering");
                state = AudioPlayerState_Buffering;
                return;
            }
            case AudioSourceState_Error: {
                printDebug("[AudioPlayer] state = AudioPlayerState_Error");
                state = AudioPlayerState_Error;
                return;
            }
            case AudioSourceState_Ready: {
                printDebug("[AudioPlayer] state = AudioPlayerState_Ready");
                state = AudioPlayerState_Ready;
                player = new Superpowered::AdvancedAudioPlayer(44100, 0);
                player->openMemory(source->getData());
                break;
            }
        }
    }
    
    if (nextState == AudioPlayerState_Initial) return;
    
    switch (nextState) {
        case AudioPlayerState_Playing: {
            if (state != AudioPlayerState_Playing) {
                if (state == AudioPlayerState_Paused) {
                    player->togglePlayback();
                } else {
                    player->play();
                    player->seek(initialPosition);
                    player->playbackRate = initialPlaybackRate;
                }
                
                printDebug("[AudioPlayer] state = AudioPlayerState_Playing");
                state = AudioPlayerState_Playing;
            }
            break;
        }
            
        case AudioPlayerState_Paused: {
            if (state != AudioPlayerState_Paused) {
                player->togglePlayback();
                printDebug("[AudioPlayer] state = AudioPlayerState_Paused");
                state = AudioPlayerState_Paused;
            }
            break;
        }
            
        default: {
            break;
        }
    }
}

bool AudioPlayer::audioProcessing(float *leftOutput, float *rightOutput, unsigned int numberOfFrames, unsigned int samplerate, bool mix, float *outputBuffer) {
    std::lock_guard<std::mutex> guard(mutex);
    
    if (state == AudioPlayerState_Playing) {
        bool hasSound = player->processStereo(outputBuffer, mix, numberOfFrames, 1);
        
        if (player->eofRecently()) {
            pause();
        }
        
        return hasSound;
    }
    
    return false;
}
