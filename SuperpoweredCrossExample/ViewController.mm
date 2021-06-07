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

    audioPlayer = [[AudioPlayer alloc] init];
}

- (void)dealloc {
#if !__has_feature(objc_arc)
    [super dealloc];
#endif
}

- (void)interruptionEnded {
}

static int number = 0;

- (IBAction)onPlayPause:(id)sender {
    if (number == 0) {
        number++;
        
        NSString* strTemplate = @"https://data.chiasenhac.com/down2/2174/1/2173432-3007e91c/flac/Butter%20Hotter%20Remix_%20-%20BTS.flac";
        
        [audioPlayer loadSource:strTemplate];
        [audioPlayer play];
    }
    else if (number == 1) {
        number++;
        
        NSString *temp = @"https://data25.chiasenhac.com/stream2/2175/1/2174506-0b096b06/flac/Doi%20La%20Cat%20Bui%20La%20Ta%20Cover_%20-%20DIMZ.flac";
        
        [audioPlayer loadSource:temp];
    }
    else if (number == 2) {
        number++;
        
        NSString *temp = @"https://data25.chiasenhac.com/stream2/2175/1/2174591-5f2e27bb/flac/Toa%20Sang%20Viet%20Nam%20-%20Miu%20Le_%20GDucky_%20Bui.flac";
        
        [audioPlayer loadSource:temp];
    }
    else if (number == 3) {
        number++;
        
        NSString* strTemplate = @"https://data.chiasenhac.com/down2/2174/1/2173432-3007e91c/flac/Butter%20Hotter%20Remix_%20-%20BTS.flac";
        
        [audioPlayer loadSource:strTemplate];
        [audioPlayer setPlaybackRate:2];
    }
    else if (number == 4) {
        number++;
        [audioPlayer setPlaybackRate:1];
    }
    else if (number == 5) {
        number++;
        NSString* strTemplate = @"https://data25.chiasenhac.com/stream2/2175/1/2174506-0b096b06/flac/Doi%20La%20Cat%20Bui%20La%20Ta%20Cover_%20-%20DIMZ.flac";
        
        audioPlayer2 = [[AudioPlayer alloc] init];
        [audioPlayer2 loadSource:strTemplate];
        [audioPlayer2 play];
    }
    else if (number == 6) {
        number++;
        audioPlayer2 = nil;
    }
    else if (number == 7) {
        number++;
        [audioPlayer seekPosition:0];
    }
}

@end
