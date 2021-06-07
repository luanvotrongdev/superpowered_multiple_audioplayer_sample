#import "ViewController.h"
#import "Wrapper.hpp"
#import "Superpowered.h"

@implementation ViewController {
    AudioPlayer *audioPlayer;
    AudioPlayer *audioPlayer2;
}

- (void)viewDidLoad {
    [super viewDidLoad];
#ifdef __IPHONE_13_0
    if (@available(iOS 13, *)) self.overrideUserInterfaceStyle = UIUserInterfaceStyleLight;
#endif
    
    Superpowered::Initialize(
                             "ABCADSADSD",
                             false, // enableAudioAnalysis (using SuperpoweredAnalyzer, SuperpoweredLiveAnalyzer, SuperpoweredWaveform or SuperpoweredBandpassFilterbank)
                             false, // enableFFTAndFrequencyDomain (using SuperpoweredFrequencyDomain, SuperpoweredFFTComplex, SuperpoweredFFTReal or SuperpoweredPolarFFT)
                             false, // enableAudioTimeStretching (using SuperpoweredTimeStretching)
                             true, // enableAudioEffects (using any SuperpoweredFX class)
                             true, // enableAudioPlayerAndDecoder (using SuperpoweredAdvancedAudioPlayer or SuperpoweredDecoder)
                             false, // enableCryptographics (using Superpowered::RSAPublicKey, Superpowered::RSAPrivateKey, Superpowered::hasher or Superpowered::AES)
                             true  // enableNetworking (using Superpowered::httpRequest)
                             );
    NSString* strTemplate = @"https://data.chiasenhac.com/down2/2174/1/2173432-3007e91c/flac/Butter%20Hotter%20Remix_%20-%20BTS.flac";
    
    audioPlayer = [[AudioPlayer alloc] init];
    [audioPlayer loadSource:strTemplate];
    [audioPlayer play];
}

- (void)dealloc {
#if !__has_feature(objc_arc)
    [super dealloc];
#endif
}

- (void)interruptionEnded {
}

- (IBAction)onPlayPause:(id)sender {
    // Test playback rate
    // [audioPlayer setPlaybackRate:3];
    
    // Test multi player
    NSString* strTemplate = @"https://data.chiasenhac.com/down2/2174/1/2173432-3007e91c/flac/Butter%20Hotter%20Remix_%20-%20BTS.flac";
    
    audioPlayer2 = [[AudioPlayer alloc] init];
    [audioPlayer2 loadSource:strTemplate];
    [audioPlayer2 play];
}

@end
