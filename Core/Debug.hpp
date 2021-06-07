#ifndef Debug_hpp
#define Debug_hpp

#if DEBUG
#define printDebug(...) do { printf(">>>> "); printf(__VA_ARGS__); printf("\n"); } while (false)
#else
#define printDebug(msg)
#endif

#endif
