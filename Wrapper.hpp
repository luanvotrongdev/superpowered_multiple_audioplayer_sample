#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface PlayerManager : NSObject

+ (instancetype) sharedInstance;

@end

@interface AudioPlayer : NSObject

- (void) loadSource: (NSString *) source;
- (void) play;
- (void) pause;
- (void) seekPosition: (double) percent;
- (void) setPlaybackRate: (double) rate;

- (double) getPosition;
- (double) getPlaybackRate;

@end

NS_ASSUME_NONNULL_END
