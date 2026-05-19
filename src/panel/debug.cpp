#include "debug.h"

static Adafruit_GFX *debug_screen = NULL;

void SetDebugScreen(Adafruit_GFX *screen)
{
    debug_screen = screen;
}

void ReportFault(const __FlashStringHelper *title, const __FlashStringHelper *message)
{
    if (!debug_screen) return;

    debug_screen->fillRect(0, 0, debug_screen->width() - 1, debug_screen->height() - 1, 0x4800);
    debug_screen->setTextColor(0xffe0);
    debug_screen->setTextSize(2);
    debug_screen->setCursor(0, 0);
    debug_screen->println(title);

    debug_screen->setTextSize(1);
    debug_screen->println(F(""));
    debug_screen->print(message);

#if !defined(MOCK_ARDUINO)
    // Halt further processing
    for (;;) { }
#endif
}

void ReportAssertionFailure(const __FlashStringHelper *condition, const __FlashStringHelper *file, unsigned int line)
{
    if (!debug_screen) return;

    debug_screen->fillRect(0, 0, debug_screen->width() - 1, debug_screen->height() - 1, 0x4800);
    debug_screen->setTextColor(0xffe0);
    debug_screen->setTextSize(2);
    debug_screen->setCursor(0, 0);
    debug_screen->println(F("ASSERT FAILED"));

    debug_screen->setTextSize(1);
    debug_screen->println(F(""));
    debug_screen->println(condition);
    debug_screen->println(F(""));
    debug_screen->print(F("at "));
    debug_screen->print(file);
    debug_screen->print(F("("));
    debug_screen->print(line);
    debug_screen->println(F(")"));

#if !defined(MOCK_ARDUINO)
    // Halt further processing
    for (;;) { }
#endif
}
