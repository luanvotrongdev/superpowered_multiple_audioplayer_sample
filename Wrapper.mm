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
    BOOL initialPlaying;
    double initialPosition;
    double initialPlaybackRate;
    NSString *source;
}

- (instancetype) init
{
    self = [super init];
    
    if (self) {
        [PlayerManager sharedInstance];
        
        self->initialPosition = 0;
        self->initialPlaybackRate = 1;
        
        typedef int (*Callback)(id, SEL, std::shared_ptr<audio::AudioPlayer>);
        
        SEL callbackSEL = @selector(setPlayer:);
        Callback callback = (Callback) [self methodForSelector:callbackSEL];
        
        audioManager.createPlayer(std::bind(callback, self, callbackSEL, std::placeholders::_1));
    }
    
    return self;
}

- (void) setPlayer: (std::shared_ptr<audio::AudioPlayer>) player
{
    self->player = player;
    
    // TODO: Improve HERE, below code can't run in this thread?????
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^(void) {
        if (self->source != nil) {
            self->player->loadSource([self->source UTF8String]);
        }
        if (self->initialPlaying) {
            self->player->play();
        }
        
        self->player->seekPosition(self->initialPosition);
        self->player->setPlaybackRate(self->initialPlaybackRate);
    });
}

- (void) loadSource: (NSString *) source
{
    self->source = source;
    
    if (player) {
        player->loadSource([source UTF8String]);
    }
}

- (void) play
{
    initialPlaying = TRUE;
    
    if (player) {
        player->play();
    }
}

- (void) pause
{
    initialPlaying = FALSE;
    
    if (player) {
        player->pause();
    }
}

- (void) seekPosition: (double) percent
{
    initialPosition = percent;
    
    if (player) {
        player->seekPosition(percent);
    }
}

- (void) setPlaybackRate: (double) rate
{
    initialPlaybackRate = rate;
    
    if (player) {
        player->setPlaybackRate(rate);
    }
}

- (double) getPosition
{
    if (player) {
        return player->getPosition();
    } else {
        return initialPosition;
    }
}

- (double) getPlaybackRate
{
    if (player) {
        return player->getPlaybackRate();
    } else {
        return initialPlaybackRate;
    }
}

@end
