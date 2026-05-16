/*
    Helper for developing and testing Arduino code on a PC
*/

#if !defined(NOTARDUINO_H) && !defined(ARDUINO)
#define NOTARDUINO_H 1

#include <cstdio>
#define TRACE(...) { \
    printf("[%d] ", millis()); \
    printf(__VA_ARGS__); \
    printf("\n"); \
}

// TODO: Check values of these
//#define OUTPUT  0
//#define INPUT   1

#define LOW     0
#define HIGH    1

// TODO: Renumber these
#define A0      100
#define A1      101
#define A2      102
#define A3      103
#define A4      104
#define A5      105

// TODO: Check types
inline void pinMode(int pin, int mode)
{ }

// TODO: Check types
inline void digitalWrite(int pin, int state)
{ }

// For testing on Windows
#if defined(WIN32)
    #include <windows.h>

    inline unsigned long millis()
    {
        return GetTickCount();
    }

    inline void delay(int duration)
    {
        Sleep(duration);
    }

    inline void delayMicroseconds(int duration)
    { }
#endif

#endif
