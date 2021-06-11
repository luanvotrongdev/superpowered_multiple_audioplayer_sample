#include "PlayerManager.hpp"

#import "Wrapper.hpp"
#import "SuperpoweredIOSAudioIO.h"

audio::PlayerManager audioManager;

static bool audioProcessing(void *clientdata, float **inputBuffers, unsigned int inputChannels, float **outputBuffers, unsigned int outputChannels, unsigned int numberOfFrames, unsigned int samplerate, uint64_t hostTime) {
    return audioManager.audioProcessing(outputBuffers[0], outputBuffers[1], numberOfFrames, samplerate);
}

// MARK: - PlayerManager

@implementation PlayerManager {
    SuperpoweredIOSAudioIO *output;
}

- (instancetype) init {
    [self doesNotRecognizeSelector:_cmd];
    return nil;
}

- (instancetype) initPrivate {
    self = [super init];
    
    if (self) {
        self->output = [[SuperpoweredIOSAudioIO alloc] initWithDelegate:(id<SuperpoweredIOSAudioIODelegate>)self preferredBufferSize:12 preferredSamplerate:44100 audioSessionCategory:AVAudioSessionCategoryPlayback channels:2 audioProcessingCallback:audioProcessing clientdata:(__bridge void *)self];
        
        [output start];
    }
    
    return self;
}

+ (instancetype) sharedInstance {
    static PlayerManager *sharedInstance;
    static dispatch_once_t onceToken;
    
    dispatch_once(&onceToken, ^{
        sharedInstance = [[self alloc] initPrivate];
    });
    
    return sharedInstance;
}

@end

// MARK: - AudioPlayer

@implementation AudioPlayer {
    std::shared_ptr<audio::AudioPlayer> player;
}

- (instancetype) init
{
    self = [super init];
    
    if (self) {
        [PlayerManager sharedInstance];
        self->player = audioManager.createPlayer();
    }
    
    return self;
}

- (void) loadSource: (NSString *) source
{
    player->setSource(audioManager.loadSource([source UTF8String]));
}

- (void) play
{
    player->play();
}

- (void) pause
{
    player->pause();
}

- (void) seekPosition: (double) percent
{
    player->seekPosition(percent);
}

- (void) setPlaybackRate: (double) rate
{
    player->setPlaybackRate(rate);
}

- (double) getPosition
{
    return player->getPosition();
}

- (double) getPlaybackRate
{
    return player->getPlaybackRate();
}

@end
