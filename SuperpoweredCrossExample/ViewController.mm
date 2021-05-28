#import "ViewController.h"
#import "Superpowered.h"
#import "SuperpoweredAdvancedAudioPlayer.h"
#import "SuperpoweredSimple.h"
#import "SuperpoweredIOSAudioIO.h"
#include <vector>



#define HEADROOM_DECIBEL 3.0f

// This is a .mm file, meaning it's Objective-C++. You can perfectly mix it with Objective-C or Swift, until you keep the member variables and C++ related includes here.
@implementation ViewController {
    SuperpoweredIOSAudioIO *output;
    std::vector<Superpowered::AdvancedAudioPlayer*> players;
    std::vector<void*> audioSources;
    Superpowered::AdvancedAudioPlayer *_player;
    Superpowered::httpRequest *request;
    
    char* linkUrl;
}

static bool audioProcessing(void *clientdata, float **inputBuffers, unsigned int inputChannels, float **outputBuffers, unsigned int outputChannels, unsigned int numberOfFrames, unsigned int samplerate, uint64_t hostTime) {
    __unsafe_unretained ViewController *self = (__bridge ViewController *)clientdata;
    return [self audioProcessing:outputBuffers[0] right:outputBuffers[1] numFrames:numberOfFrames samplerate:samplerate];
}

static bool onRequestCallback (void *clientData,
                               Superpowered::httpRequest *_request,
                               Superpowered::httpResponse *_response)
{
    ViewController* view = (__bridge ViewController*)clientData;
    printf("onRequestCallback %s %s", (char*)clientData, Superpowered::statusCodeToString(_response->statusCode, "UNDEFINED"));
    if(_response->statusCode == Superpowered::httpResponse::StatusCode_Success)
    {
        char *data = new char[_response->dataOrFileSizeBytes];
        memcpy(data, _response->data, _response->dataOrFileSizeBytes);
        view->audioSources.push_back(Superpowered::Decoder::decodeToAudioInMemory(data, _response->dataOrFileSizeBytes));
        
        return false;
    }
    return true;
}

static void onRequestLogCallback(void *clientData, const char *str)
{
    ViewController* view = (__bridge ViewController*)clientData;
    printf("onRequestLogCallback %s %s", view->linkUrl, str);
}

// This is where the Superpowered magic happens.
- (bool)audioProcessing:(float *)leftOutput right:(float *)rightOutput numFrames:(unsigned int)numberOfFrames samplerate:(unsigned int)samplerate {
    int size = players.size();
    if(size == 0)
        return false;
    // Get audio from the players into a buffer on the stack.
    float outputBuffer[numberOfFrames * 2];
    bool hasSound = false;
    bool isFirstPlay = true;
    for(int i=0;i<size;i++)
    {
        Superpowered::AdvancedAudioPlayer* player = players[i];
        if(player->isPlaying())
        {
            hasSound |= player->processStereo(outputBuffer, isFirstPlay ? false : true, numberOfFrames, 1);
            isFirstPlay = false;
        }
        if(player->eofRecently())
        {
            player->pause();
        }
        
        
    }
    // The output buffer is ready now, let's put the finished audio into the left and right outputs.
    if (hasSound) Superpowered::DeInterleave(outputBuffer, leftOutput, rightOutput, numberOfFrames);
    return hasSound;
}

- (void)viewDidLoad {
    [super viewDidLoad];
#ifdef __IPHONE_13_0
    if (@available(iOS 13, *)) self.overrideUserInterfaceStyle = UIUserInterfaceStyleLight;
#endif
    
    Superpowered::Initialize(
                             "SUPERPOWERED_KEY",
                             false, // enableAudioAnalysis (using SuperpoweredAnalyzer, SuperpoweredLiveAnalyzer, SuperpoweredWaveform or SuperpoweredBandpassFilterbank)
                             false, // enableFFTAndFrequencyDomain (using SuperpoweredFrequencyDomain, SuperpoweredFFTComplex, SuperpoweredFFTReal or SuperpoweredPolarFFT)
                             false, // enableAudioTimeStretching (using SuperpoweredTimeStretching)
                             true, // enableAudioEffects (using any SuperpoweredFX class)
                             true, // enableAudioPlayerAndDecoder (using SuperpoweredAdvancedAudioPlayer or SuperpoweredDecoder)
                             false, // enableCryptographics (using Superpowered::RSAPublicKey, Superpowered::RSAPrivateKey, Superpowered::hasher or Superpowered::AES)
                             true  // enableNetworking (using Superpowered::httpRequest)
                             );
    NSString* strTemplate = @"https://mepofficial.s3.amazonaws.com/game_sfx/beat_blader/05b89b35-0584-4f86-ab5b-100651079bed/%d.mp3";
    for(int i=1;i<=4;i++)
    {
        NSString* url = [NSString stringWithFormat:strTemplate, i];
        request = new Superpowered::httpRequest([url UTF8String]);
        request->sendAsync(onRequestCallback,
                           (__bridge void*)self,
                           NULL,
                           false,
                           onRequestLogCallback);
    }
    
    output = [[SuperpoweredIOSAudioIO alloc] initWithDelegate:(id<SuperpoweredIOSAudioIODelegate>)self preferredBufferSize:12 preferredSamplerate:44100 audioSessionCategory:AVAudioSessionCategoryPlayback channels:2 audioProcessingCallback:audioProcessing clientdata:(__bridge void *)self];
    [output start];
}

- (void)dealloc {
//    delete player;
#if !__has_feature(objc_arc)
    [output release];
    [super dealloc];
#endif
}

- (void)interruptionEnded { // If a player plays Apple Lossless audio files, then we need this. Otherwise unnecessary.
//    player->onMediaserverInterrupt();
}

- (IBAction)onPlayPause:(id)sender {
    if(audioSources.size() == 0)
        return;
    int idx = arc4random_uniform(audioSources.size());
    Superpowered::AdvancedAudioPlayer* player = [self getPlayer];
    player->openMemory(audioSources[idx]);
    player->play();
}

- (Superpowered::AdvancedAudioPlayer*)getPlayer{
    for(int i=0;i<players.size();i++)
    {
        if(!players[i]->isPlaying())
            return players[i];
    }
    Superpowered::AdvancedAudioPlayer* player = new Superpowered::AdvancedAudioPlayer(44100, 0);
    players.push_back(player);
    
    return player;
}

@end
