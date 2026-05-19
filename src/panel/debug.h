#ifndef SCRAPBRAIN_DEBUG_H
#define SCRAPBRAIN_DEBUG_H 1

#include <Adafruit_GFX.h>

#if WITH_ASSERT == 1
    #define ASSERT(condition) \
        { if (!(condition)) { ReportAssertionFailure(F(#condition), F(__FILE__), __LINE__); } }
#else
    #define ASSERT(condition)
#endif

void SetDebugScreen(Adafruit_GFX *screen);

void ReportFault(const __FlashStringHelper *title, const __FlashStringHelper *message);

void ReportAssertionFailure(const __FlashStringHelper *condition, const __FlashStringHelper *file, unsigned int line);

#endif
