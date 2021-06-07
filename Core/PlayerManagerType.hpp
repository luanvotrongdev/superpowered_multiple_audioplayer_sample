#ifndef PlayerManagerType_h
#define PlayerManagerType_h

#include <string>

namespace audio {

class PlayerManagerType {
public:
    virtual void notifyStateChanged() = 0;
    virtual void preloadSource(const std::string &source) = 0;
};

}

#endif /* PlayerManagerType_h */
