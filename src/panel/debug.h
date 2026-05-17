#ifndef SCRAPBRAIN_DEBUG_H
#define SCRAPBRAIN_DEBUG_H 1

#include <Adafruit_GFX.h>

#if defined(DEBUG)
    #define ASSERT(condition) \
        { if (!(condition)) { ReportAssertionFailure(#condition, __FILE__, __LINE__); } }
#else
    #define ASSERT(condition)
#endif

void SetDebugScreen(Adafruit_GFX *screen);

void ReportFault(const char *title, const char *message);

void ReportAssertionFailure(const char *condition, const char *file, unsigned int line);

#endif
