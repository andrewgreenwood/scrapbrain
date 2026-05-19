/*
    SCRAP BRAIN - YM2612 Hardware Synth - Panel
    Author: Andrew Greenwood

    TODO: Implement mux and potentiometer reading

    Hardware: Atmega328p

    Pin map (Arduino):
        0   Serial RX (MIDI in), also connected to FTDI TX
        1   Serial TX (MIDI out) also connected to FTDI RX
        2   Display TCS
        3   Display DC
        4   Multiplexer S0
        5   Multiplexer S1
        6   Multiplexer S2
        7   Multiplexer S3 (only for 16-channel, not main panel)
        8   -
        9   Display backlight
        10  -
        11  SPI MOSI (for touchscreen)
        12  SPI MISO (for touchscreen)
        13  SPI SCK (for touchscreen)
        A0  Multiplexer 3 COM
        A1  Multiplexer 1 COM
        A2  Multiplexer 2 COM
        A3  Main panel multiplexer COM
        A4  I2C SDA (for touchscreen)
        A5  I2C SCL (for touchscreen)
*/

// Debugging options
#define WITH_ASSERT             1
#define WITH_HOTSPOT_OUTLINE    1
#define WITH_DEBUG_PAGE         1

#define DISPLAY_TCS_PIN         2
#define DISPLAY_DC_PIN          3
#define DISPLAY_BACKLIGHT_PIN   9

#define MUX_S0_PIN              4
#define MUX_S1_PIN              5
#define MUX_S2_PIN              6
#define MUX_S3_PIN              7

#define MAIN_MUX_COM_PIN        A3
#define MUX_1_COM_PIN           A1
#define MUX_2_COM_PIN           A2
#define MUX_3_COM_PIN           A0

#include <Arduino.h>

#ifdef MOCK_ARDUINO
#include "mockscreen.h"
#else
#include <EEPROM.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_FT6206.h>
#include <MIDI.h>
#endif

#include "panelkit.h"
#include "debug.h"

#ifdef MOCK_ARDUINO
MockScreen screen(240, 320);
MockTouchScreen touchscreen;
#else
struct MidiSettings : public midi::DefaultSettings {
    static const unsigned SysExMaxSize = 1;
    static const long BaudRate = 31250;
};
midi::SerialMIDI<HardwareSerial, MidiSettings> serialMIDI(Serial);
midi::MidiInterface<midi::SerialMIDI<HardwareSerial, MidiSettings>, MidiSettings> MIDI((midi::SerialMIDI<HardwareSerial, MidiSettings>&)serialMIDI);
Adafruit_ILI9341 screen(DISPLAY_TCS_PIN, DISPLAY_DC_PIN);
Adafruit_FT6206 touchscreen;
#endif


#define EEPROM_SETTINGS_OFFSET  0x000
#define EEPROM_PATCHES_OFFSET   sizeof(settings)

#define EEPROM_SIGNATURE    0xa9

struct Settings {
    uint8_t signature;
    uint8_t midi_channel;
} settings;

#define BACKGROUND_COLOUR           0x1082
#define TOPBAR_COLOUR               0x0000
#define CARRIER_OPERATOR_COLOUR     0x2589
#define MODULATOR_OPERATOR_COLOUR   0xf3e4
#define OPERATOR_LINK_COLOUR        0xffff

#define INDICATOR_OUTLINE_COLOUR    0xffff
#define GREEN_INDICATOR_COLOUR      0x2589
#define GREEN_INDICATOR_LOW_COLOUR  0x0000

#define BUTTON_OUTLINE_COLOUR       BACKGROUND_COLOUR
// 0x0841

enum {
    GRAPHIC_INDICATOR_OUTLINE = GRAPHIC_FIRST_ID,
    GRAPHIC_GREEN_INDICATOR,
    GRAPHIC_ALGORITHM_BUTTON_OUTLINE,
    GRAPHIC_CARRIER_OPERATOR_SQUARE,
    GRAPHIC_BIG_CARRIER_OPERATOR_SQUARE,
    GRAPHIC_MODULATOR_OPERATOR_SQUARE,
    GRAPHIC_BIG_MODULATOR_OPERATOR_SQUARE,
    GRAPHIC_VERTICAL_OPERATOR_CONNECTION,
    GRAPHIC_BIG_VERTICAL_OPERATOR_CONNECTION,
    GRAPHIC_UP_LEFT_OPERATOR_CONNECTION,
    GRAPHIC_BIG_UP_LEFT_OPERATOR_CONNECTION,
    GRAPHIC_UP_RIGHT_OPERATOR_CONNECTION,
    GRAPHIC_BIG_UP_RIGHT_OPERATOR_CONNECTION,
    GRAPHIC_LEFT_UP_OPERATOR_CONNECTION,
    GRAPHIC_BIG_LEFT_UP_OPERATOR_CONNECTION,
    GRAPHIC_RIGHT_UP_OPERATOR_CONNECTION,
    GRAPHIC_BIG_RIGHT_UP_OPERATOR_CONNECTION,
    GRAPHIC_ALGORITHM_1_SMALL,
    GRAPHIC_ALGORITHM_1_BIG,
    GRAPHIC_ALGORITHM_2_SMALL,
    GRAPHIC_ALGORITHM_2_BIG,
    GRAPHIC_ALGORITHM_3_SMALL,
    GRAPHIC_ALGORITHM_3_BIG,
    GRAPHIC_ALGORITHM_4_SMALL,
    GRAPHIC_ALGORITHM_4_BIG,
    GRAPHIC_ALGORITHM_5_SMALL,
    GRAPHIC_ALGORITHM_5_BIG,
    GRAPHIC_ALGORITHM_6_SMALL,
    GRAPHIC_ALGORITHM_6_BIG,
    GRAPHIC_ALGORITHM_7_SMALL,
    GRAPHIC_ALGORITHM_7_BIG,
    GRAPHIC_ALGORITHM_8_SMALL,
    GRAPHIC_ALGORITHM_8_BIG,
    GRAPHIC_FOLDER_IMAGE,
    GRAPHIC_COG_IMAGE,
    GRAPHIC_BOX_IMAGE,
    GRAPHIC_UP_ARROW_IMAGE,
    GRAPHIC_DOWN_ARROW_IMAGE,
    GRAPHIC_RESET_ARROW_IMAGE,
    GRAPHIC_OPEN_FOLDER_IMAGE,
    GRAPHIC_MIDI_CONNECTOR,
    GRAPHIC_LEFT_CHEVRON,
    GRAPHIC_RIGHT_CHEVRON,
    GRAPHIC_SMALL_BUTTON_OUTLINE,
    GRAPHIC_ALGORITHM_SELECT
};

BEGIN_GRAPHIC(graphicIndicatorOutlineData)
    GRAPHIC_SET_COLOUR(0xffff)
    GRAPHIC_HORIZONTAL_LINE(0, 1, 5)
    GRAPHIC_HORIZONTAL_LINE(6, 1, 5)
    GRAPHIC_VERTICAL_LINE(0, 1, 5)
    GRAPHIC_VERTICAL_LINE(6, 1, 5)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicGreenIndicatorData)
    GRAPHIC_SET_COLOUR(GREEN_INDICATOR_COLOUR)
    GRAPHIC_FILLED_SQUARE(1, 1, 3)
    GRAPHIC_HORIZONTAL_LINE(0, 0, 4)
    GRAPHIC_VERTICAL_LINE(0, 1, 4)
    GRAPHIC_SET_COLOUR(GREEN_INDICATOR_LOW_COLOUR)
    GRAPHIC_HORIZONTAL_LINE(4, 1, 4)
    GRAPHIC_VERTICAL_LINE(4, 1, 3)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicAlgorithmButtonOutlineData)
    GRAPHIC_SET_COLOUR(BUTTON_OUTLINE_COLOUR)
    GRAPHIC_SQUARE(0, 0, 45)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicCarrierOperatorSquareData)
    GRAPHIC_SET_COLOUR(CARRIER_OPERATOR_COLOUR)
    GRAPHIC_SQUARE(0, 0, 7)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicBigCarrierOperatorSquareData)
    GRAPHIC_SET_COLOUR(CARRIER_OPERATOR_COLOUR)
    GRAPHIC_SQUARE(0, 0, 30)
    GRAPHIC_SQUARE(1, 1, 28)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicModulatorOperatorSquareData)
    GRAPHIC_SET_COLOUR(MODULATOR_OPERATOR_COLOUR)
    GRAPHIC_SQUARE(0, 0, 7)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicBigModulatorOperatorSquareData)
    GRAPHIC_SET_COLOUR(MODULATOR_OPERATOR_COLOUR)
    GRAPHIC_SQUARE(0, 0, 30)
    GRAPHIC_SQUARE(1, 1, 28)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicVerticalOperatorConnectionData)
    GRAPHIC_SET_COLOUR(OPERATOR_LINK_COLOUR)
    GRAPHIC_VERTICAL_LINE(0, 0, 2)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicBigVerticalOperatorConnectionData)
    GRAPHIC_SET_COLOUR(OPERATOR_LINK_COLOUR)
    GRAPHIC_FILLED_RECTANGLE(0, 0, 3, 10)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicUpLeftOperatorConnectionData)
    GRAPHIC_SET_COLOUR(OPERATOR_LINK_COLOUR)
    GRAPHIC_VERTICAL_LINE(3, 1, 5)
    GRAPHIC_HORIZONTAL_LINE(0, 0, 2)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicBigUpLeftOperatorConnectionData)
    GRAPHIC_SET_COLOUR(OPERATOR_LINK_COLOUR)
    GRAPHIC_FILLED_RECTANGLE(0, 0, 16, 3)
    GRAPHIC_FILLED_RECTANGLE(14, 1, 3, 22)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicUpRightOperatorConnectionData)
    GRAPHIC_SET_COLOUR(OPERATOR_LINK_COLOUR)
    GRAPHIC_VERTICAL_LINE(0, 1, 5)
    GRAPHIC_HORIZONTAL_LINE(0, 1, 3)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicBigUpRightOperatorConnectionData)
    GRAPHIC_SET_COLOUR(OPERATOR_LINK_COLOUR)
    GRAPHIC_FILLED_RECTANGLE(1, 0, 15, 3)
    GRAPHIC_FILLED_RECTANGLE(0, 1, 3, 22)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicLeftUpOperatorConnectionData)
    GRAPHIC_SET_COLOUR(OPERATOR_LINK_COLOUR)
    GRAPHIC_HORIZONTAL_LINE(5, 1, 6)
    GRAPHIC_VERTICAL_LINE(0, 0, 4)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicBigLeftUpOperatorConnectionData)
    GRAPHIC_SET_COLOUR(OPERATOR_LINK_COLOUR)
    GRAPHIC_FILLED_RECTANGLE(1, 20, 26, 3)
    GRAPHIC_FILLED_RECTANGLE(0, 0, 3, 22)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicRightUpOperatorConnectionData)
    GRAPHIC_SET_COLOUR(OPERATOR_LINK_COLOUR)
    GRAPHIC_HORIZONTAL_LINE(5, 0, 5)
    GRAPHIC_VERTICAL_LINE(6, 0, 4)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicBigRightUpOperatorConnectionData)
    GRAPHIC_SET_COLOUR(OPERATOR_LINK_COLOUR)
    GRAPHIC_FILLED_RECTANGLE(0, 20, 26, 3)
    GRAPHIC_FILLED_RECTANGLE(24, 0, 3, 22)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicAlgorithm1SmallData)
    GRAPHIC(GRAPHIC_CARRIER_OPERATOR_SQUARE, 17, 2)
    GRAPHIC(GRAPHIC_MODULATOR_OPERATOR_SQUARE, 17, 12)
    GRAPHIC(GRAPHIC_MODULATOR_OPERATOR_SQUARE, 17, 22)
    GRAPHIC(GRAPHIC_MODULATOR_OPERATOR_SQUARE, 17, 32)
    GRAPHIC(GRAPHIC_VERTICAL_OPERATOR_CONNECTION, 20, 9)
    GRAPHIC(GRAPHIC_VERTICAL_OPERATOR_CONNECTION, 20, 19)
    GRAPHIC(GRAPHIC_VERTICAL_OPERATOR_CONNECTION, 20, 29)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicAlgorithm1BigData)
    GRAPHIC_SET_TEXT_SIZE(2)
    GRAPHIC(GRAPHIC_BIG_CARRIER_OPERATOR_SQUARE, 60, 0)
    GRAPHIC_CHARACTER(70, 8, '4')
    GRAPHIC(GRAPHIC_BIG_MODULATOR_OPERATOR_SQUARE, 60, 40)
    GRAPHIC_CHARACTER(70, 48, '3')
    GRAPHIC(GRAPHIC_BIG_MODULATOR_OPERATOR_SQUARE, 60, 80)
    GRAPHIC_CHARACTER(70, 88, '2')
    GRAPHIC(GRAPHIC_BIG_MODULATOR_OPERATOR_SQUARE, 60, 120)
    GRAPHIC_CHARACTER(70, 128, '1')
    GRAPHIC(GRAPHIC_BIG_VERTICAL_OPERATOR_CONNECTION, 74, 30)
    GRAPHIC(GRAPHIC_BIG_VERTICAL_OPERATOR_CONNECTION, 74, 70)
    GRAPHIC(GRAPHIC_BIG_VERTICAL_OPERATOR_CONNECTION, 74, 110)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicAlgorithm2SmallData)
    GRAPHIC(GRAPHIC_CARRIER_OPERATOR_SQUARE, 17, 7)
    GRAPHIC(GRAPHIC_MODULATOR_OPERATOR_SQUARE, 17, 17)
    GRAPHIC(GRAPHIC_MODULATOR_OPERATOR_SQUARE, 10, 27)
    GRAPHIC(GRAPHIC_MODULATOR_OPERATOR_SQUARE, 24, 27)
    GRAPHIC(GRAPHIC_VERTICAL_OPERATOR_CONNECTION, 20, 14)
    GRAPHIC(GRAPHIC_UP_RIGHT_OPERATOR_CONNECTION, 13, 21)
    GRAPHIC(GRAPHIC_UP_LEFT_OPERATOR_CONNECTION, 24, 21)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicAlgorithm2BigData)
    GRAPHIC_SET_TEXT_SIZE(2)
    GRAPHIC(GRAPHIC_BIG_CARRIER_OPERATOR_SQUARE, 60, 20)
    GRAPHIC_CHARACTER(70, 28, '4')
    GRAPHIC(GRAPHIC_BIG_MODULATOR_OPERATOR_SQUARE, 60, 60)
    GRAPHIC_CHARACTER(70, 68, '3')
    GRAPHIC(GRAPHIC_BIG_MODULATOR_OPERATOR_SQUARE, 30, 100)
    GRAPHIC_CHARACTER(40, 108, '1')
    GRAPHIC(GRAPHIC_BIG_MODULATOR_OPERATOR_SQUARE, 90, 100)
    GRAPHIC_CHARACTER(100, 108, '2')
    GRAPHIC(GRAPHIC_BIG_VERTICAL_OPERATOR_CONNECTION, 74, 50)
    GRAPHIC(GRAPHIC_BIG_UP_RIGHT_OPERATOR_CONNECTION, 44, 77)
    GRAPHIC(GRAPHIC_BIG_UP_LEFT_OPERATOR_CONNECTION, 90, 77)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicAlgorithm3SmallData)
    GRAPHIC(GRAPHIC_CARRIER_OPERATOR_SQUARE, 17, 7)
    GRAPHIC(GRAPHIC_MODULATOR_OPERATOR_SQUARE, 10, 17)
    GRAPHIC(GRAPHIC_MODULATOR_OPERATOR_SQUARE, 24, 17)
    GRAPHIC(GRAPHIC_MODULATOR_OPERATOR_SQUARE, 24, 27)
    GRAPHIC(GRAPHIC_UP_RIGHT_OPERATOR_CONNECTION, 13, 11)
    GRAPHIC(GRAPHIC_UP_LEFT_OPERATOR_CONNECTION, 24, 11)
    GRAPHIC(GRAPHIC_VERTICAL_OPERATOR_CONNECTION, 27, 24)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicAlgorithm3BigData)
    GRAPHIC_SET_TEXT_SIZE(2)
    GRAPHIC(GRAPHIC_BIG_CARRIER_OPERATOR_SQUARE, 60, 20)
    GRAPHIC_CHARACTER(70, 28, '4')
    GRAPHIC(GRAPHIC_BIG_MODULATOR_OPERATOR_SQUARE, 30, 60)
    GRAPHIC_CHARACTER(40, 68, '1')
    GRAPHIC(GRAPHIC_BIG_MODULATOR_OPERATOR_SQUARE, 90, 60)
    GRAPHIC_CHARACTER(100, 68, '3')
    GRAPHIC(GRAPHIC_BIG_MODULATOR_OPERATOR_SQUARE, 90, 100)
    GRAPHIC_CHARACTER(100, 108, '2')
    GRAPHIC(GRAPHIC_BIG_UP_RIGHT_OPERATOR_CONNECTION, 44, 37)
    GRAPHIC(GRAPHIC_BIG_UP_LEFT_OPERATOR_CONNECTION, 90, 37)
    GRAPHIC(GRAPHIC_BIG_VERTICAL_OPERATOR_CONNECTION, 104, 90)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicAlgorithm4SmallData)
    GRAPHIC(GRAPHIC_CARRIER_OPERATOR_SQUARE, 17, 7)
    GRAPHIC(GRAPHIC_MODULATOR_OPERATOR_SQUARE, 10, 17)
    GRAPHIC(GRAPHIC_MODULATOR_OPERATOR_SQUARE, 24, 17)
    GRAPHIC(GRAPHIC_MODULATOR_OPERATOR_SQUARE, 10, 27)
    GRAPHIC(GRAPHIC_UP_RIGHT_OPERATOR_CONNECTION, 13, 11)
    GRAPHIC(GRAPHIC_UP_LEFT_OPERATOR_CONNECTION, 24, 11)
    GRAPHIC(GRAPHIC_VERTICAL_OPERATOR_CONNECTION, 13, 24)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicAlgorithm4BigData)
    GRAPHIC_SET_TEXT_SIZE(2)
    GRAPHIC(GRAPHIC_BIG_CARRIER_OPERATOR_SQUARE, 60, 20)
    GRAPHIC_CHARACTER(70, 28, '4')
    GRAPHIC(GRAPHIC_BIG_MODULATOR_OPERATOR_SQUARE, 30, 60)
    GRAPHIC_CHARACTER(40, 68, '2')
    GRAPHIC(GRAPHIC_BIG_MODULATOR_OPERATOR_SQUARE, 90, 60)
    GRAPHIC_CHARACTER(100, 68, '3')
    GRAPHIC(GRAPHIC_BIG_MODULATOR_OPERATOR_SQUARE, 30, 100)
    GRAPHIC_CHARACTER(40, 108, '1')
    GRAPHIC(GRAPHIC_BIG_UP_RIGHT_OPERATOR_CONNECTION, 44, 37)
    GRAPHIC(GRAPHIC_BIG_UP_LEFT_OPERATOR_CONNECTION, 90, 37)
    GRAPHIC(GRAPHIC_BIG_VERTICAL_OPERATOR_CONNECTION, 44, 90)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicAlgorithm5SmallData)
    GRAPHIC(GRAPHIC_CARRIER_OPERATOR_SQUARE, 12, 12)
    GRAPHIC(GRAPHIC_CARRIER_OPERATOR_SQUARE, 22, 12)
    GRAPHIC(GRAPHIC_MODULATOR_OPERATOR_SQUARE, 12, 22)
    GRAPHIC(GRAPHIC_MODULATOR_OPERATOR_SQUARE, 22, 22)
    GRAPHIC(GRAPHIC_VERTICAL_OPERATOR_CONNECTION, 15, 19)
    GRAPHIC(GRAPHIC_VERTICAL_OPERATOR_CONNECTION, 25, 19)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicAlgorithm5BigData)
    GRAPHIC_SET_TEXT_SIZE(2)
    GRAPHIC(GRAPHIC_BIG_CARRIER_OPERATOR_SQUARE, 40, 40)
    GRAPHIC_CHARACTER(50, 48, '2')
    GRAPHIC(GRAPHIC_BIG_CARRIER_OPERATOR_SQUARE, 80, 40)
    GRAPHIC_CHARACTER(90, 48, '4')
    GRAPHIC(GRAPHIC_BIG_MODULATOR_OPERATOR_SQUARE, 40, 80)
    GRAPHIC_CHARACTER(50, 88, '1')
    GRAPHIC(GRAPHIC_BIG_MODULATOR_OPERATOR_SQUARE, 80, 80)
    GRAPHIC_CHARACTER(90, 88, '3')
    GRAPHIC(GRAPHIC_BIG_VERTICAL_OPERATOR_CONNECTION, 54, 70)
    GRAPHIC(GRAPHIC_BIG_VERTICAL_OPERATOR_CONNECTION, 94, 70)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicAlgorithm6SmallData)
    GRAPHIC(GRAPHIC_CARRIER_OPERATOR_SQUARE, 7, 12)
    GRAPHIC(GRAPHIC_CARRIER_OPERATOR_SQUARE, 17, 12)
    GRAPHIC(GRAPHIC_CARRIER_OPERATOR_SQUARE, 27, 12)
    GRAPHIC(GRAPHIC_MODULATOR_OPERATOR_SQUARE, 17, 22)
    GRAPHIC(GRAPHIC_LEFT_UP_OPERATOR_CONNECTION, 10, 19)
    GRAPHIC(GRAPHIC_VERTICAL_OPERATOR_CONNECTION, 20, 19)
    GRAPHIC(GRAPHIC_RIGHT_UP_OPERATOR_CONNECTION, 24, 19)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicAlgorithm6BigData)
    GRAPHIC_SET_TEXT_SIZE(2)
    GRAPHIC(GRAPHIC_BIG_CARRIER_OPERATOR_SQUARE, 20, 40)
    GRAPHIC_CHARACTER(30, 48, '2')
    GRAPHIC(GRAPHIC_BIG_CARRIER_OPERATOR_SQUARE, 60, 40)
    GRAPHIC_CHARACTER(70, 48, '3')
    GRAPHIC(GRAPHIC_BIG_CARRIER_OPERATOR_SQUARE, 100, 40)
    GRAPHIC_CHARACTER(110, 48, '4')
    GRAPHIC(GRAPHIC_BIG_MODULATOR_OPERATOR_SQUARE, 60, 80)
    GRAPHIC_CHARACTER(70, 88, '1')
    GRAPHIC(GRAPHIC_BIG_VERTICAL_OPERATOR_CONNECTION, 74, 70)
    GRAPHIC(GRAPHIC_BIG_LEFT_UP_OPERATOR_CONNECTION, 33, 70)
    GRAPHIC(GRAPHIC_BIG_RIGHT_UP_OPERATOR_CONNECTION, 90, 70)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicAlgorithm7SmallData)
    GRAPHIC(GRAPHIC_CARRIER_OPERATOR_SQUARE, 7, 12)
    GRAPHIC(GRAPHIC_CARRIER_OPERATOR_SQUARE, 17, 12)
    GRAPHIC(GRAPHIC_CARRIER_OPERATOR_SQUARE, 27, 12)
    GRAPHIC(GRAPHIC_MODULATOR_OPERATOR_SQUARE, 7, 22)
    GRAPHIC(GRAPHIC_VERTICAL_OPERATOR_CONNECTION, 10, 19)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicAlgorithm7BigData)
    GRAPHIC_SET_TEXT_SIZE(2)
    GRAPHIC(GRAPHIC_BIG_CARRIER_OPERATOR_SQUARE, 20, 40)
    GRAPHIC_CHARACTER(30, 48, '2')
    GRAPHIC(GRAPHIC_BIG_CARRIER_OPERATOR_SQUARE, 60, 40)
    GRAPHIC_CHARACTER(70, 48, '3')
    GRAPHIC(GRAPHIC_BIG_CARRIER_OPERATOR_SQUARE, 100, 40)
    GRAPHIC_CHARACTER(110, 48, '4')
    GRAPHIC(GRAPHIC_BIG_MODULATOR_OPERATOR_SQUARE, 20, 80)
    GRAPHIC_CHARACTER(30, 88, '1')
    GRAPHIC(GRAPHIC_BIG_VERTICAL_OPERATOR_CONNECTION, 34, 70)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicAlgorithm8SmallData)
    GRAPHIC(GRAPHIC_CARRIER_OPERATOR_SQUARE, 2, 17)
    GRAPHIC(GRAPHIC_CARRIER_OPERATOR_SQUARE, 12, 17)
    GRAPHIC(GRAPHIC_CARRIER_OPERATOR_SQUARE, 22, 17)
    GRAPHIC(GRAPHIC_CARRIER_OPERATOR_SQUARE, 32, 17)
END_GRAPHIC()

BEGIN_GRAPHIC(graphicAlgorithm8BigData)
    GRAPHIC_SET_TEXT_SIZE(2)
    GRAPHIC(GRAPHIC_BIG_CARRIER_OPERATOR_SQUARE, 0, 60)
    GRAPHIC_CHARACTER(10, 68, '1')
    GRAPHIC(GRAPHIC_BIG_CARRIER_OPERATOR_SQUARE, 40, 60)
    GRAPHIC_CHARACTER(50, 68, '2')
    GRAPHIC(GRAPHIC_BIG_CARRIER_OPERATOR_SQUARE, 80, 60)
    GRAPHIC_CHARACTER(90, 68, '3')
    GRAPHIC(GRAPHIC_BIG_CARRIER_OPERATOR_SQUARE, 120, 60)
    GRAPHIC_CHARACTER(130, 68, '4')
END_GRAPHIC()

BEGIN_GRAPHIC(folderGraphicData)
    //GRAPHIC_SET_COLOUR(COLOUR_YELLOW)
    GRAPHIC_BITMAP(0, 0, 24, 21,
        0x7f, 0x00, 0x00,
        0x80, 0x80, 0x00,
        0x80, 0x40, 0x00,
        0x80, 0x3f, 0xfe,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x80, 0x00, 0x01,
        0x7f, 0xff, 0xfe
    )
END_GRAPHIC()

BEGIN_GRAPHIC(cogGraphicData)
    //GRAPHIC_SET_COLOUR(COLOUR_YELLOW)
    GRAPHIC_BITMAP(0, 0, 21, 21,
        0x00, 0x70, 0x00,
        0x00, 0x88, 0x00,
        0x0c, 0x89, 0x80,
        0x13, 0x06, 0x40,
        0x20, 0x00, 0x20,
        0x20, 0x00, 0x20,
        0x10, 0x00, 0x40,
        0x10, 0x70, 0x40,
        0x60, 0x88, 0x30,
        0x81, 0x04, 0x08,
        0x81, 0x04, 0x08,
        0x81, 0x04, 0x08,
        0x60, 0x88, 0x30,
        0x10, 0x70, 0x40,
        0x10, 0x00, 0x40,
        0x20, 0x00, 0x20,
        0x20, 0x00, 0x20,
        0x13, 0x06, 0x40,
        0x0c, 0x89, 0x80,
        0x00, 0x88, 0x00,
        0x00, 0x70, 0x00
    )
END_GRAPHIC()

BEGIN_GRAPHIC(boxGraphicData)
    //GRAPHIC_SET_COLOUR(COLOUR_YELLOW)
    GRAPHIC_BITMAP(0, 0, 34, 25,
        0x00, 0x01, 0x04, 0x00, 0x00,
        0x00, 0x0e, 0x03, 0x00, 0x00,
        0x00, 0x70, 0x00, 0xc0, 0x00,
        0x03, 0x80, 0x00, 0x30, 0x00,
        0x0c, 0x00, 0x00, 0x1c, 0x00,
        0x13, 0x00, 0x00, 0x0f, 0x00,
        0x20, 0xc0, 0x00, 0x74, 0xc0,
        0x40, 0x30, 0x03, 0x84, 0x00,
        0x80, 0x0c, 0x1c, 0x04, 0x00,
        0xc0, 0x03, 0xe0, 0x04, 0x00,
        0x30, 0x02, 0x80, 0x04, 0x00,
        0x1c, 0x04, 0x80, 0x04, 0x00,
        0x13, 0x08, 0x80, 0x04, 0x00,
        0x10, 0xd0, 0x80, 0x04, 0x00,
        0x10, 0x20, 0x80, 0x04, 0x00,
        0x10, 0x00, 0x80, 0x04, 0x00,
        0x10, 0x00, 0x80, 0x04, 0x00,
        0x10, 0x00, 0x80, 0x04, 0x00,
        0x10, 0x00, 0x80, 0x04, 0x00,
        0x0c, 0x00, 0x80, 0x04, 0x00,
        0x03, 0x00, 0x80, 0x0c, 0x00,
        0x00, 0xc0, 0x80, 0x70, 0x00,
        0x00, 0x30, 0x83, 0x80, 0x00,
        0x00, 0x0c, 0x9c, 0x00, 0x00,
        0x00, 0x03, 0xe0, 0x00, 0x00
    )
END_GRAPHIC()

BEGIN_GRAPHIC(arrowUpGraphicData)
    //GRAPHIC_SET_COLOUR(COLOUR_WHITE)
    GRAPHIC_BITMAP(0, 0, 13, 16,
        0x07, 0x00,
        0x08, 0x80,
        0x10, 0x40,
        0x20, 0x20,
        0x40, 0x10,
        0x80, 0x08,
        0x88, 0x88,
        0x98, 0xc8,
        0x68, 0xb0,
        0x08, 0x80,
        0x08, 0x80,
        0x08, 0x80,
        0x08, 0x80,
        0x08, 0x80,
        0x08, 0x80,
        0x07, 0x00
    )
END_GRAPHIC()

BEGIN_GRAPHIC(arrowDownGraphicData)
    //GRAPHIC_SET_COLOUR(COLOUR_WHITE)
    GRAPHIC_BITMAP(0, 0, 13, 16,
        0x07, 0x00,
        0x08, 0x80,
        0x08, 0x80,
        0x08, 0x80,
        0x08, 0x80,
        0x08, 0x80,
        0x08, 0x80,
        0x68, 0xb0,
        0x98, 0xc8,
        0x88, 0x88,
        0x80, 0x08,
        0x40, 0x10,
        0x20, 0x20,
        0x10, 0x40,
        0x08, 0x80,
        0x07, 0x00
    )
END_GRAPHIC()

BEGIN_GRAPHIC(arrowResetGraphicData)
    //GRAPHIC_SET_COLOUR(COLOUR_YELLOW)
    GRAPHIC_BITMAP(0, 0, 25, 29,
        0x01, 0xc0, 0x00, 0x00,
        0x02, 0x20, 0x00, 0x00,
        0x04, 0x20, 0x00, 0x00,
        0x08, 0x40, 0x00, 0x00,
        0x10, 0xfe, 0x00, 0x00,
        0x20, 0x01, 0xc0, 0x00,
        0x20, 0x00, 0x30, 0x00,
        0x20, 0x00, 0x08, 0x00,
        0x10, 0xfe, 0x04, 0x00,
        0x08, 0x41, 0x82, 0x00,
        0x04, 0x20, 0x42, 0x00,
        0x02, 0x20, 0x21, 0x00,
        0x01, 0xc0, 0x11, 0x00,
        0x00, 0x00, 0x11, 0x00,
        0x00, 0x00, 0x08, 0x80,
        0x70, 0x00, 0x08, 0x80,
        0x88, 0x00, 0x08, 0x80,
        0x88, 0x00, 0x08, 0x80,
        0x88, 0x00, 0x08, 0x80,
        0x44, 0x00, 0x11, 0x00,
        0x44, 0x00, 0x11, 0x00,
        0x42, 0x00, 0x21, 0x00,
        0x21, 0x00, 0x42, 0x00,
        0x20, 0xc1, 0x82, 0x00,
        0x10, 0x3e, 0x04, 0x00,
        0x08, 0x00, 0x08, 0x00,
        0x06, 0x00, 0x30, 0x00,
        0x01, 0xc1, 0xc0, 0x00,
        0x00, 0x3e, 0x00, 0x00
    )
END_GRAPHIC()

BEGIN_GRAPHIC(folderOpenGraphicData)
    //GRAPHIC_SET_COLOUR(COLOUR_YELLOW)
    GRAPHIC_BITMAP(0, 0, 29, 21,
        0x7f, 0x00, 0x00, 0x00,
        0x80, 0x80, 0x00, 0x00,
        0x80, 0x40, 0x00, 0x00,
        0x80, 0x3c, 0x1e, 0x00,
        0x80, 0x00, 0x01, 0x00,
        0x80, 0x00, 0x01, 0x00,
        0x80, 0x00, 0x01, 0x00,
        0x81, 0xff, 0xff, 0xf0,
        0x82, 0x00, 0x00, 0x08,
        0x82, 0x00, 0x00, 0x08,
        0x84, 0x00, 0x00, 0x10,
        0x84, 0x00, 0x00, 0x10,
        0x88, 0x00, 0x00, 0x20,
        0x88, 0x00, 0x00, 0x20,
        0x90, 0x00, 0x00, 0x40,
        0x90, 0x00, 0x00, 0x40,
        0xa0, 0x00, 0x00, 0x80,
        0xa0, 0x00, 0x00, 0x80,
        0xc0, 0x00, 0x01, 0x00,
        0x40, 0x00, 0x01, 0x00,
        0x3f, 0xff, 0xfe, 0x00
    )
END_GRAPHIC()

BEGIN_GRAPHIC(midiConnectorGraphic)
    //GRAPHIC_SET_COLOUR(COLOUR_YELLOW)
    GRAPHIC_BITMAP(0, 0, 32, 33,
        0x00, 0x0f, 0xf0, 0x00,
        0x00, 0x37, 0xec, 0x00,
        0x00, 0xc7, 0xe3, 0x00,
        0x03, 0x07, 0xe0, 0xc0,
        0x04, 0x07, 0xe0, 0x20,
        0x08, 0x00, 0x00, 0x10,
        0x08, 0x00, 0x00, 0x08,
        0x10, 0x00, 0x00, 0x08,
        0x20, 0x00, 0x00, 0x04,
        0x20, 0x00, 0x00, 0x04,
        0x40, 0x00, 0x00, 0x02,
        0x40, 0x00, 0x00, 0x02,
        0x40, 0x00, 0x00, 0x02,
        0x80, 0x00, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x01,
        0x83, 0x00, 0x00, 0xc1,
        0x83, 0x00, 0x00, 0xc1,
        0x80, 0x00, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x01,
        0x40, 0x00, 0x00, 0x02,
        0x40, 0x00, 0x00, 0x02,
        0x40, 0x60, 0x06, 0x02,
        0x20, 0x60, 0x06, 0x04,
        0x20, 0x00, 0x00, 0x04,
        0x10, 0x01, 0x80, 0x08,
        0x08, 0x01, 0x80, 0x08,
        0x08, 0x00, 0x00, 0x10,
        0x04, 0x00, 0x00, 0x20,
        0x03, 0x00, 0x00, 0xc0,
        0x00, 0xc0, 0x03, 0x00,
        0x00, 0x30, 0x0c, 0x00,
        0x00, 0x0f, 0xf0, 0x00
    )
END_GRAPHIC()

BEGIN_GRAPHIC(leftChevronGraphic)
    GRAPHIC_BITMAP(0, 0, 8, 14,
        0x03,
        0x07,
        0x0e,
        0x1c,
        0x38,
        0x70,
        0xe0,
        0xe0,
        0x70,
        0x38,
        0x1c,
        0x0e,
        0x07,
        0x03
    )
END_GRAPHIC()

BEGIN_GRAPHIC(rightChevronGraphic)
    GRAPHIC_BITMAP(0, 0, 8, 14,
        0xc0,
        0xe0,
        0x70,
        0x38,
        0x1c,
        0x0e,
        0x07,
        0x07,
        0x0e,
        0x1c,
        0x38,
        0x70,
        0xe0,
        0xc0
    )
END_GRAPHIC()

BEGIN_GRAPHIC(smallButtonOutlineGraphic)
    GRAPHIC_SET_COLOUR(COLOUR_WHITE)        // TODO
    GRAPHIC_VERTICAL_LINE(0, 2, 23)
    GRAPHIC_HORIZONTAL_LINE(0, 2, 23)
    GRAPHIC_VERTICAL_LINE(25, 2, 23)
    GRAPHIC_HORIZONTAL_LINE(25, 2, 23)
    GRAPHIC_PIXEL(1, 1)
    GRAPHIC_PIXEL(24, 1)
    GRAPHIC_PIXEL(1, 24)
    GRAPHIC_PIXEL(24, 24)
END_GRAPHIC()

BEGIN_GRAPHIC(algorithmSelectGraphic)
    GRAPHIC_SET_COLOUR(COLOUR_WHITE)        // TODO
    GRAPHIC_VERTICAL_LINE(0, 0, 5)
    GRAPHIC_HORIZONTAL_LINE(0, 1, 5)
    GRAPHIC_VERTICAL_LINE(36, 0, 5)
    GRAPHIC_HORIZONTAL_LINE(0, 31, 35)
    GRAPHIC_VERTICAL_LINE(0, 31, 36)
    GRAPHIC_HORIZONTAL_LINE(36, 1, 5)
    GRAPHIC_VERTICAL_LINE(36, 31, 36)
    GRAPHIC_HORIZONTAL_LINE(36, 31, 35)
END_GRAPHIC()

const uint8_t* const graphics[] PROGMEM = {
    graphicIndicatorOutlineData,
    graphicGreenIndicatorData,
    graphicAlgorithmButtonOutlineData,
    graphicCarrierOperatorSquareData,
    graphicBigCarrierOperatorSquareData,
    graphicModulatorOperatorSquareData,
    graphicBigModulatorOperatorSquareData,
    graphicVerticalOperatorConnectionData,
    graphicBigVerticalOperatorConnectionData,
    graphicUpLeftOperatorConnectionData,
    graphicBigUpLeftOperatorConnectionData,
    graphicUpRightOperatorConnectionData,
    graphicBigUpRightOperatorConnectionData,
    graphicLeftUpOperatorConnectionData,
    graphicBigLeftUpOperatorConnectionData,
    graphicRightUpOperatorConnectionData,
    graphicBigRightUpOperatorConnectionData,
    graphicAlgorithm1SmallData,
    graphicAlgorithm1BigData,
    graphicAlgorithm2SmallData,
    graphicAlgorithm2BigData,
    graphicAlgorithm3SmallData,
    graphicAlgorithm3BigData,
    graphicAlgorithm4SmallData,
    graphicAlgorithm4BigData,
    graphicAlgorithm5SmallData,
    graphicAlgorithm5BigData,
    graphicAlgorithm6SmallData,
    graphicAlgorithm6BigData,
    graphicAlgorithm7SmallData,
    graphicAlgorithm7BigData,
    graphicAlgorithm8SmallData,
    graphicAlgorithm8BigData,
    folderGraphicData,
    cogGraphicData,
    boxGraphicData,
    arrowUpGraphicData,
    arrowDownGraphicData,
    arrowResetGraphicData,
    folderOpenGraphicData,
    midiConnectorGraphic,
    leftChevronGraphic,
    rightChevronGraphic,
    smallButtonOutlineGraphic,
    algorithmSelectGraphic
};


//
// The title bar and MIDI indicator state
//
class TopBar: public Panel {
    public:
        TopBar(UI &ui)
        : Panel(ui, 0, 0, 320, 24) { }

        virtual ~TopBar() { }

        void setMidiIndicatorState(bool state)
        {
            int16_t x = 273;

            if (state) {
                drawGraphic(GRAPHIC_GREEN_INDICATOR, x, 9);
            } else {
                setColour(BACKGROUND_COLOUR);
                fillRectangle(x, 9, 5, 5);
            }
        }

    private:
        virtual void draw()
        {
            fill(TOPBAR_COLOUR);
            setTextSize(1);
            setColour(COLOUR_WHITE);

            // Title
            setCursor(12, 8);
            print(F("Scrap Brain YM2612"));

            // MIDI indicator
            drawGraphic(GRAPHIC_INDICATOR_OUTLINE, 272, 8);
            setCursor(285, 8);
            print(F("MIDI"));
        }

        virtual void onTouchEvent(TouchEventType type, uint8_t hotspot_id, int16_t x, int16_t y) { }

        virtual void process() { }
};

class Page;

class Pager {
    friend class Page;

    public:
        Pager(UI &ui, int16_t x, int16_t y, int16_t width, int16_t height)
        : m_ui(ui), m_x(x), m_y(y), m_width(width), m_height(height),
          m_current_page(NULL)
        {
        }

        virtual ~Pager()
        {
        }

        void setPage(Page &page);

        bool isCurrentPage(Page &page)
        { return m_current_page == &page; }

    private:
        UI &m_ui;
        int16_t m_x;
        int16_t m_y;
        int16_t m_width;
        int16_t m_height;
        Page *m_current_page;
};

class Page: public Panel {
    friend class Pager;

    public:
        Page(Pager &pager)
        : Panel(pager.m_ui, pager.m_x, pager.m_y, pager.m_width, pager.m_height),
          m_number_of_hotspots(0), m_hotspots(NULL) { }

        virtual ~Page() { }

    protected:
        virtual void onEnter() = 0;
        virtual void onLeave() = 0;

        virtual void setHotspots(uint16_t count, const Hotspot hotspots[])
        {
            m_number_of_hotspots = count;
            m_hotspots = hotspots;
        }

    private:
        virtual void draw() { }

        int16_t m_number_of_hotspots;
        const Hotspot *m_hotspots;
};

void Pager::setPage(Page &page)
{
    if (m_current_page) {
        m_current_page->hide();
        m_current_page->onLeave();
    }
    m_current_page = &page;
    page.Panel::setHotspots(page.m_number_of_hotspots, page.m_hotspots);
    m_current_page->onEnter();
    m_current_page->show();
}

class NotificationPage: public Page {
    public:
        NotificationPage(Pager &pager)
        : Page(pager), m_colour(COLOUR_WHITE), m_message(NULL), m_expiry_time(0), m_animate_start_time(0), m_next_page(NULL)
        { }

        void notify(uint16_t colour, const __FlashStringHelper *message, uint16_t duration, Page &next_page)
        {
            m_colour = colour;
            m_message = message;
            m_expiry_time = millis() + duration;
            m_next_page = &next_page;
        }

    private:
        virtual void onEnter()
        { m_animate_start_time = millis(); }

        virtual void onLeave() { }

        virtual void draw()
        {
            uint16_t x = (width() / 2) - ((strlen_P(reinterpret_cast<PGM_P>(m_message)) * 12) / 2);
            if (millis() - m_animate_start_time < 100) {
                setColour(scaleColour(m_colour, 0x80));
            } else {
                setColour(m_colour);
            }
            setTextSize(2);
            setCursor(x, (height() / 2) - 14);
            print(m_message);
        }

        virtual void onTouchEvent(TouchEventType type, uint8_t hotspot_id, int16_t x, int16_t y) { }

        virtual void process();

        uint16_t m_colour;
        const __FlashStringHelper *m_message;
        unsigned long m_expiry_time;
        unsigned long m_animate_start_time;
        Page *m_next_page;
};


#define PAGE_HEADER_Y   11

class PatchSelectPage: public Page {
    public:
        enum Mode {
            Factory,
            Load,
            Save
        };

        PatchSelectPage(Pager &pager)
        : Page(pager), m_mode(Factory), m_patch(-1)
        {
            setHotspots(NumberOfHotspots, s_hotspots);
        }

        virtual ~PatchSelectPage() { }

        virtual void draw()
        {
            setColour(COLOUR_WHITE);
            drawGraphic(GRAPHIC_SMALL_BUTTON_OUTLINE, 20, PAGE_HEADER_Y);
            drawGraphic(GRAPHIC_LEFT_CHEVRON, 28, PAGE_HEADER_Y + 6);
            setTextSize(2);
            drawText(65, PAGE_HEADER_Y + 5, m_mode == Save ? F("Save Patch") : F("Load Patch"));

            drawCurrentPatchSelection();
        }

        // Must be called before selecting the page
        void setMode(Mode mode)
        {
            m_mode = mode;            
        }

        void drawCurrentPatchSelection()
        {
            int i;
            char label[3] = {0, 0, 0};

            setTextSize(2);

            for (i = 0; i < 5; ++ i) {
                setColour(m_patch == i ? COLOUR_BRIGHT_GREEN : COLOUR_WHITE);

                label[0] = '0';
                label[1] = '1' + i;
                drawRectangle(25 + (i * 57), 79, 42, 42);
                drawText(35 + (i * 57), 92, label);

                setColour(m_patch == 5 + i ? COLOUR_BRIGHT_GREEN : COLOUR_GREY);       // TODO: this demos unoccupied slots

                label[0] = i == 4 ? '1' : '0';
                label[1] = i == 4 ? '0' : '6' + i;
                drawRectangle(25 + (i * 57), 139, 42, 42);
                drawText(35 + (i * 57), 152, label);
            }
        }

    private:
        virtual void onEnter() { }
        virtual void onLeave() { }

        virtual void onTouchEvent(TouchEventType type, uint8_t hotspot_id, int16_t x, int16_t y);

        virtual void process() { }

        Mode m_mode;

        int8_t m_patch;

        enum {
            BackButtonHotspotId = 1,
            Patch1ButtonHotspotId,
            Patch2ButtonHotspotId,
            Patch3ButtonHotspotId,
            Patch4ButtonHotspotId,
            Patch5ButtonHotspotId,
            Patch6ButtonHotspotId,
            Patch7ButtonHotspotId,
            Patch8ButtonHotspotId,
            Patch9ButtonHotspotId,
            Patch10ButtonHotspotId,
            NumberOfHotspots
        };

        static const Hotspot PROGMEM s_hotspots[NumberOfHotspots];
};

const Hotspot PROGMEM PatchSelectPage::s_hotspots[PatchSelectPage::NumberOfHotspots] = {
    { .id = PatchSelectPage::BackButtonHotspotId,    .x = 0,   .y = 0,   .width = 70, .height = 50  },
    { .id = PatchSelectPage::Patch1ButtonHotspotId,  .x = 21,  .y = 75,  .width = 50, .height = 50  },
    { .id = PatchSelectPage::Patch2ButtonHotspotId,  .x = 78,  .y = 75,  .width = 50, .height = 50  },
    { .id = PatchSelectPage::Patch3ButtonHotspotId,  .x = 135, .y = 75,  .width = 50, .height = 50  },
    { .id = PatchSelectPage::Patch4ButtonHotspotId,  .x = 192, .y = 75,  .width = 50, .height = 50  },
    { .id = PatchSelectPage::Patch5ButtonHotspotId,  .x = 249, .y = 75,  .width = 50, .height = 50  },
    { .id = PatchSelectPage::Patch6ButtonHotspotId,  .x = 21,  .y = 135, .width = 50, .height = 50  },
    { .id = PatchSelectPage::Patch7ButtonHotspotId,  .x = 78,  .y = 135, .width = 50, .height = 50  },
    { .id = PatchSelectPage::Patch8ButtonHotspotId,  .x = 135, .y = 135, .width = 50, .height = 50  },
    { .id = PatchSelectPage::Patch9ButtonHotspotId,  .x = 192, .y = 135, .width = 50, .height = 50  },
    { .id = PatchSelectPage::Patch10ButtonHotspotId, .x = 249, .y = 135, .width = 50, .height = 50  }
};

/*
class PatchLoadPage: public PatchSelectPage {
    public:
        PatchLoadPage(Pager &pager)
        : PatchSelectPage(pager)
        {
        }

    private:
        virtual void drawTitle(int x, int y)
        {
            drawText(x, y, "Load Patch");
        }

        virtual void onPatchSelected(uint8_t patch)
        {
        }
};

class PatchSavePage: public PatchSelectPage {
    public:
        PatchSavePage(Pager &pager)
        : PatchSelectPage(pager)
        {
        }

    private:
        virtual void drawTitle(int x, int y)
        {
            drawText(x, y, "Save Patch");
        }

        virtual void onPatchSelected(uint8_t patch)
        {
        }
};
*/


class PatchOptionsPage: public Page {
    public:
        PatchOptionsPage(Pager &pager)
        : Page(pager)
        {
            setHotspots(NumberOfPageHotspots, s_hotspots);
        }

        virtual void draw()
        {
            setColour(COLOUR_WHITE);
            drawGraphic(GRAPHIC_SMALL_BUTTON_OUTLINE, 20, PAGE_HEADER_Y);
            drawGraphic(GRAPHIC_LEFT_CHEVRON, 28, PAGE_HEADER_Y + 6);
            setTextSize(2);
            drawText(65, PAGE_HEADER_Y + 5, F("Patch"));

            setTextSize(1);
            setColour(COLOUR_WHITE);
            drawText(55, 125, F("Reset"));
            setColour(COLOUR_YELLOW);
            drawGraphic(GRAPHIC_RESET_ARROW_IMAGE, 58, 87);

            setColour(COLOUR_WHITE);
            drawText(109, 125, F("Factory"));
            drawText(109, 138, F("Patches"));
            drawGraphic(GRAPHIC_UP_ARROW_IMAGE, 125, 81);
            setColour(COLOUR_YELLOW);
            drawGraphic(GRAPHIC_BOX_IMAGE, 113, 90);

            setColour(COLOUR_WHITE);
            drawText(178, 125, F("Load"));
            drawGraphic(GRAPHIC_UP_ARROW_IMAGE, 185, 81);
            setColour(COLOUR_YELLOW);
            drawGraphic(GRAPHIC_OPEN_FOLDER_IMAGE, 175, 92);

            setColour(COLOUR_WHITE);
            drawText(238, 125, F("Save"));
            drawGraphic(GRAPHIC_DOWN_ARROW_IMAGE, 245, 81);
            setColour(COLOUR_YELLOW);
            drawGraphic(GRAPHIC_OPEN_FOLDER_IMAGE, 235, 92);
        }

    private:
        virtual void onEnter() { }
        virtual void onLeave() { }

        virtual void onTouchEvent(TouchEventType type, uint8_t hotspot_id, int16_t x, int16_t y);

        virtual void process() { }

        enum {
            BackButtonHotspotId = 1,
            ResetButtonHotspotId,
            FactoryButtonHotspotId,
            LoadButtonHotspotId,
            SaveButtonHotspotId,
            NumberOfPageHotspots
        };

    static const Hotspot PROGMEM s_hotspots[NumberOfPageHotspots];
};

const Hotspot PROGMEM PatchOptionsPage::s_hotspots[PatchOptionsPage::NumberOfPageHotspots] = {
    { .id = PatchOptionsPage::BackButtonHotspotId,     .x = 0,   .y = 0,  .width = 70, .height = 50 },
    { .id = PatchOptionsPage::ResetButtonHotspotId,    .x = 45,  .y = 75, .width = 50, .height = 75 },
    { .id = PatchOptionsPage::FactoryButtonHotspotId,  .x = 105, .y = 75, .width = 50, .height = 75 },
    { .id = PatchOptionsPage::LoadButtonHotspotId,     .x = 165, .y = 75, .width = 50, .height = 75 },
    { .id = PatchOptionsPage::SaveButtonHotspotId,     .x = 225, .y = 75, .width = 50, .height = 75 }
};


#if WITH_DEBUG_PAGE == 1
class DebugPage: public Page {
    public:
        DebugPage(Pager &pager)
        : Page(pager)
        {
            setHotspots(NumberOfHotspots, s_hotspots);
        }

        virtual void draw()
        {
            // Page heading and back button
            setColour(COLOUR_WHITE);
            drawGraphic(GRAPHIC_SMALL_BUTTON_OUTLINE, 20, PAGE_HEADER_Y);
            drawGraphic(GRAPHIC_LEFT_CHEVRON, 28, PAGE_HEADER_Y + 6);
            setTextSize(2);
            drawText(65, PAGE_HEADER_Y + 5, F("Debug"));

            // Buttons
            setTextSize(1);
            setColour(COLOUR_BRIGHT_GREEN);
            drawRectangle(47, 52, 46, 46);
            drawText(59, 70, F("Note"));

            // Spare buttons
            setColour(COLOUR_DARK_GREY);
            drawRectangle(105, 50, 50, 50);
            //drawText(34, 70, "-");

            drawRectangle(165, 50, 50, 50);
            //drawText(34, 70, "-");

            drawRectangle(225, 50, 50, 50);
//            drawText(34, 70, "-");

            // Control readings
            setColour(COLOUR_WHITE);
            setTextSize(1);
            drawText(14, 115, F("Control Readings"));
            setTextSize(1);
            setColour(COLOUR_BRIGHT_GREEN);
            drawText(14, 130,  F("  00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15"));
            drawText(14, 145, "A");
            drawText(14, 160, "B");
            drawText(14, 175, "C");
            drawText(14, 190, "D");

            // Default (0xFF = undefined)
            for (int mux = 0; mux < 4; ++ mux) {
                for (int control = 0; control < 16; ++ control) {
                    if ((mux < 3) || (control < 8)) {
                        updateControlValue(mux, control, 0xFF);
                    }
                }
            }

            updatePotReadTimeMeasurement(0);
        }

        void updateControlValue(uint8_t mux, uint8_t channel, uint8_t value)
        {
            ASSERT(mux < 4);
            ASSERT(channel < 16);
            ASSERT((mux < 3) || (channel < 8));

            int x = 26 + (channel * 18);
            int y = 145 + (mux * 15);
            setColour(COLOUR_BLACK);
            fillRectangle(x - 1, y - 1, 14, 10);
            setColour(COLOUR_YELLOW);
            setCursor(x, y);
            if (value == 0xff) {
                print(F("--"));
            } else {
                if (value < 0x10) print("0");
                print(value, HEX);
            }
        }

        void updatePotReadTimeMeasurement(uint16_t time)
        {
            setColour(COLOUR_BLACK);
            fillRectangle(259, 189, 26, 10);

            setTextSize(1);
            setCursor(260, 190);
            setColour(COLOUR_BRIGHT_BLUE);
            print(time);
        }

    private:
        virtual void onEnter() { }
        virtual void onLeave() { }

        virtual void onTouchEvent(TouchEventType type, uint8_t hotspot_id, int16_t x, int16_t y);

        virtual void process() { }

        enum {
            BackButtonHotspotId = 1,
            NoteTriggerButtonId,
            NumberOfHotspots
        };

        static const Hotspot PROGMEM s_hotspots[NumberOfHotspots];
};

const Hotspot PROGMEM DebugPage::s_hotspots[DebugPage::NumberOfHotspots] = {
    { .id = DebugPage::BackButtonHotspotId, .x = 0,   .y = 0,  .width = 70, .height = 50 },
    { .id = DebugPage::NoteTriggerButtonId, .x = 45,  .y = 50, .width = 50, .height = 50 }
};
#endif


class SettingsPage: public Page {
    public:
        SettingsPage(Pager &pager)
        : Page(pager), m_reload_settings(true), m_midi_channel(0)
        {
            setHotspots(NumberOfPageHotspots, s_hotspots);
            // TODO: Read from EEPROM
        }

        virtual ~SettingsPage() { }

        virtual void draw()
        {
            setColour(COLOUR_WHITE);
            drawGraphic(GRAPHIC_SMALL_BUTTON_OUTLINE, 20, PAGE_HEADER_Y);
            drawGraphic(GRAPHIC_LEFT_CHEVRON, 28, PAGE_HEADER_Y + 6);
            setTextSize(2);
            drawText(65, PAGE_HEADER_Y + 5, F("Settings"));

            setTextSize(1);
            drawText(55, 76, F("MIDI Channel"));
            //drawText(200, 76, "Secondary");
            drawGraphic(GRAPHIC_MIDI_CONNECTOR, 75, 98);
            //drawGraphic(GRAPHIC_MIDI_CONNECTOR, 210, 98);

            drawMidiChannel();
            drawGraphic(GRAPHIC_LEFT_CHEVRON, 50, 108);
            drawGraphic(GRAPHIC_RIGHT_CHEVRON, 124, 108);

#if WITH_DEBUG_PAGE == 1
            setTextSize(1);
            drawText(275, 177, F("Debug"));
#endif
        }

        void drawMidiChannel()
        {
            setTextSize(2);
            setColour(COLOUR_WHITE);
            setCursor(80, 146);
            if (m_midi_channel < 9) print("0");
            print(m_midi_channel + 1);
        }
        
    private:
        virtual void onEnter()
        {
            if (m_reload_settings) {
                m_midi_channel = settings.midi_channel;
            }
        }

        virtual void onLeave() { }

        virtual void onTouchEvent(TouchEventType type, uint8_t hotspot_id, int16_t x, int16_t y);

        virtual void process() { }

        bool modified()
        {
            return settings.midi_channel != m_midi_channel;
        }

        void commit()
        {
            if (!modified()) return;

            settings.midi_channel = m_midi_channel;
#if !defined(MOCK_ARDUINO)
            EEPROM.put(EEPROM_SETTINGS_OFFSET, settings);
#endif
        }

        bool m_reload_settings;
        uint8_t m_midi_channel;

        enum {
            BackButtonHotspotId = 1,
            MidiChannelDecrementButtonHotspotId,
            MidiChannelIncrementButtonHotspotId,
            DebugButtonHotspotId,
            NumberOfPageHotspots
        };

        static const Hotspot PROGMEM s_hotspots[NumberOfPageHotspots];
};

const Hotspot PROGMEM SettingsPage::s_hotspots[SettingsPage::NumberOfPageHotspots] = {
    { .id = SettingsPage::BackButtonHotspotId,                  .x = 0,   .y = 0,   .width = 70, .height = 50 },
    { .id = SettingsPage::MidiChannelDecrementButtonHotspotId,  .x = 30,  .y = 90,  .width = 50, .height = 50 },
    { .id = SettingsPage::MidiChannelIncrementButtonHotspotId,  .x = 102, .y = 90,  .width = 50, .height = 50 },
    { .id = SettingsPage::DebugButtonHotspotId,                 .x = 270, .y = 160, .width = 40, .height = 40 }
};


class MainPage: public Page {
    public:
        MainPage(Pager &pager)
        : Page(pager), m_algorithm(0)
        {
            setHotspots(NumberOfHotspots, s_hotspots);
        }

        virtual void draw()
        {
            int y = 10;
            for (int i = 0; i < 4; ++ i) {
                drawGraphic(GRAPHIC_ALGORITHM_BUTTON_OUTLINE, 14, y);
                drawGraphic(GRAPHIC_ALGORITHM_BUTTON_OUTLINE, 260, y);
                y += 48;
            }

            drawGraphic(GRAPHIC_ALGORITHM_1_SMALL, 16, 12);
            drawGraphic(GRAPHIC_ALGORITHM_2_SMALL, 16, 60);
            drawGraphic(GRAPHIC_ALGORITHM_3_SMALL, 16, 108);
            drawGraphic(GRAPHIC_ALGORITHM_4_SMALL, 16, 156);

            drawGraphic(GRAPHIC_ALGORITHM_5_SMALL, 262, 12);
            drawGraphic(GRAPHIC_ALGORITHM_6_SMALL, 262, 60);
            drawGraphic(GRAPHIC_ALGORITHM_7_SMALL, 262, 108);
            drawGraphic(GRAPHIC_ALGORITHM_8_SMALL, 262, 156);

            setColour(COLOUR_YELLOW);
            drawGraphic(GRAPHIC_FOLDER_IMAGE, 95, 166);
            drawGraphic(GRAPHIC_COG_IMAGE, 201, 166);

            setTextSize(1);
            setColour(COLOUR_WHITE);
            drawText(87, 195, F("Patches"));
            drawText(188, 195, F("Settings"));

            drawCurrentAlgorithm();
        }

        void drawCurrentAlgorithm()
        {
            const uint8_t graphic_ids[8] = {
                GRAPHIC_ALGORITHM_1_BIG,
                GRAPHIC_ALGORITHM_2_BIG,
                GRAPHIC_ALGORITHM_3_BIG,
                GRAPHIC_ALGORITHM_4_BIG,
                GRAPHIC_ALGORITHM_5_BIG,
                GRAPHIC_ALGORITHM_6_BIG,
                GRAPHIC_ALGORITHM_7_BIG,
                GRAPHIC_ALGORITHM_8_BIG
            };            

            ASSERT(m_algorithm < 8);
            drawGraphic(graphic_ids[m_algorithm], 84, 12);

            int selection_x = m_algorithm < 4 ? 18 : 264;
            int selection_y = 14 + ((m_algorithm % 4) * 48);

            drawGraphic(GRAPHIC_ALGORITHM_SELECT, selection_x, selection_y);
        }

    private:
        virtual void onEnter() { }
        virtual void onLeave() { }

        virtual void onTouchEvent(TouchEventType type, uint8_t hotspot_id, int16_t x, int16_t y);

        virtual void process() { }

        uint8_t m_algorithm;

        enum {
            Algorithm1HotspotId = 1,
            Algorithm2HotspotId,
            Algorithm3HotspotId,
            Algorithm4HotspotId,
            Algorithm5HotspotId,
            Algorithm6HotspotId,
            Algorithm7HotspotId,
            Algorithm8HotspotId,
            PatchButtonHotspotId,
            SettingsButtonHotspotId,
            NumberOfHotspots
        };

        static const Hotspot PROGMEM s_hotspots[NumberOfHotspots];
};

const Hotspot PROGMEM MainPage::s_hotspots[MainPage::NumberOfHotspots] = {
    { .id = MainPage::Algorithm1HotspotId,     .x = 0,   .y = 9,   .width = 75, .height = 48 },
    { .id = MainPage::Algorithm2HotspotId,     .x = 0,   .y = 57,  .width = 75, .height = 48 },
    { .id = MainPage::Algorithm3HotspotId,     .x = 0,   .y = 105, .width = 75, .height = 48 },
    { .id = MainPage::Algorithm4HotspotId,     .x = 0,   .y = 153, .width = 75, .height = 48 },
    { .id = MainPage::Algorithm5HotspotId,     .x = 244, .y = 9,   .width = 75, .height = 48 },
    { .id = MainPage::Algorithm6HotspotId,     .x = 244, .y = 57,  .width = 75, .height = 48 },
    { .id = MainPage::Algorithm7HotspotId,     .x = 244, .y = 105, .width = 75, .height = 48 },
    { .id = MainPage::Algorithm8HotspotId,     .x = 244, .y = 153, .width = 75, .height = 48 },
    { .id = MainPage::PatchButtonHotspotId,    .x = 83,  .y = 156, .width = 48, .height = 60 },
    { .id = MainPage::SettingsButtonHotspotId, .x = 188, .y = 156, .width = 48, .height = 60 }
};


class Splash: public Panel {
    public:
        Splash(UI &ui)
        : Panel(ui, 0, 0, 320, 240) { }

        virtual void draw()
        {
            setTextSize(2);
            setCursor(100, 105);
            print(F("Scrap Brain"));
            setTextSize(1);
            setCursor(100, 125);
            print(F("YM2612 Synthesiser"));
        }

    private:
        virtual void onEnter() { }
        virtual void onLeave() { }
        virtual void onTouchEvent(TouchEventType type, uint8_t hotspot_id, int16_t x, int16_t y) { }
        virtual void process() { }
};


UI ui(screen);
TopBar top_bar(ui);
Pager pager(ui, 0, 24, 320, 216);
MainPage page(pager);
NotificationPage notification_page(pager);
PatchOptionsPage patch_options_page(pager);
PatchSelectPage patch_select_page(pager);
SettingsPage settings_page(pager);

#if WITH_DEBUG_PAGE == 1
DebugPage debug_page(pager);
#endif


void NotificationPage::process()
{
    if (m_expiry_time < millis() + 100) {
        m_animate_start_time = millis();
        draw();
    }

    if (millis() - m_animate_start_time > 100) {
        draw();
    }

    if (millis() > m_expiry_time) {
        pager.setPage(*m_next_page);
    }
}


void MainPage::onTouchEvent(TouchEventType type, uint8_t hotspot_id, int16_t x, int16_t y)
{
    if ((type == TouchStartEvent) || (type == TouchMoveEvent)) {
        if ((hotspot_id >= Algorithm1HotspotId) && (hotspot_id <= Algorithm8HotspotId)) {
            if (m_algorithm != hotspot_id - Algorithm1HotspotId) {
                forceBackgroundColour(true);
                drawCurrentAlgorithm();
                m_algorithm = hotspot_id - Algorithm1HotspotId;
                forceBackgroundColour(false);
                drawCurrentAlgorithm();
            }
        }
    } else if (type == TouchTapEvent) {
        switch (hotspot_id) {
            case PatchButtonHotspotId:
                pager.setPage(patch_options_page);
                break;

            case SettingsButtonHotspotId:
                pager.setPage(settings_page);
                break;
        };
    }
}

void PatchOptionsPage::onTouchEvent(TouchEventType type, uint8_t hotspot_id, int16_t x, int16_t y)
{
    // TODO: Separate subclasses for factory/load/save
    if (type == TouchTapEvent) {
        switch (hotspot_id) {
            case BackButtonHotspotId:
                pager.setPage(page);
                break;

            case ResetButtonHotspotId:
                // TODO
                notification_page.notify(COLOUR_BRIGHT_GREEN, F("Patch has been reset"), 2000, page);
                pager.setPage(notification_page);
                break;

            case FactoryButtonHotspotId:
                patch_select_page.setMode(PatchSelectPage::Factory);
                pager.setPage(patch_select_page);
                break;

            case LoadButtonHotspotId:
                patch_select_page.setMode(PatchSelectPage::Load);
                pager.setPage(patch_select_page);
                break;

            case SaveButtonHotspotId:
                patch_select_page.setMode(PatchSelectPage::Save);
                pager.setPage(patch_select_page);
                break;
        };
    }
}

void PatchSelectPage::onTouchEvent(TouchEventType type, uint8_t hotspot_id, int16_t x, int16_t y)
{
    if (type == TouchTapEvent) {
        switch (hotspot_id) {
            case BackButtonHotspotId:
                pager.setPage(patch_options_page);
                break;

            default:
                if ((hotspot_id >= Patch1ButtonHotspotId) && (hotspot_id <= Patch10ButtonHotspotId)) {
                    m_patch = hotspot_id - Patch1ButtonHotspotId;
                    ASSERT(m_patch < 10);
                    drawCurrentPatchSelection();
                }
                break;
        }
    }
}

void SettingsPage::onTouchEvent(TouchEventType type, uint8_t hotspot_id, int16_t x, int16_t y)
{
    if (type == TouchTapEvent) {
        switch (hotspot_id) {
            case BackButtonHotspotId:
                if (modified()) {
                    commit();
#if !defined(MOCK_ARDUINO)
                    //EEPROM.put(EEPROM_SETTINGS_OFFSET, settings);
#endif
                    notification_page.notify(COLOUR_BRIGHT_GREEN, F("Settings saved"), 2000, page);
                    pager.setPage(notification_page);
                } else {
                    pager.setPage(page);
                }
                m_reload_settings = true;
                break;

            case DebugButtonHotspotId:
#if WITH_DEBUG_PAGE == 1
                pager.setPage(debug_page);
#endif
                m_reload_settings = false;
                break;
        };
    } else if (type == TouchStartEvent) {
        switch (hotspot_id) {
            case MidiChannelDecrementButtonHotspotId:
                if (m_midi_channel > 0) {
                    forceBackgroundColour(true);
                    drawMidiChannel();
                    forceBackgroundColour(false);
                    -- m_midi_channel;
                    drawMidiChannel();
                }
                break;

            case MidiChannelIncrementButtonHotspotId:
                if (m_midi_channel < 15) {
                    forceBackgroundColour(true);
                    drawMidiChannel();
                    forceBackgroundColour(false);
                    ++ m_midi_channel;
                    drawMidiChannel();
                }
                break;
        };
    }
}

#if WITH_DEBUG_PAGE == 1
void DebugPage::onTouchEvent(TouchEventType type, uint8_t hotspot_id, int16_t x, int16_t y)
{
    if ((type == TouchTapEvent) && (hotspot_id == BackButtonHotspotId)) {
        pager.setPage(settings_page);
    } else if (type == TouchStartEvent) {
    }
}
#endif

#define MIDI_INDICATOR_BLINK_TIME   250
unsigned long last_midi_event_time = 0;

#define POT_READING_BUFFER_SIZE     4
uint8_t pot_readings[56][POT_READING_BUFFER_SIZE];
uint8_t previous_pot_readings[56];      // TODO: init to ff
uint8_t pot_reading_index = 0;

static void selectMuxChannel(uint8_t channel)
{
#if !defined(MOCK_ARDUINO)
    PORTD &= 0x0f;
    PORTD |= channel << 4;
#endif
}

uint8_t getMostCommonPotReading(uint8_t pot_index)
{
    uint8_t most_popular = 0;
    int most_popular_count = 0;

    for (int i = 0; i < POT_READING_BUFFER_SIZE; ++ i) {
        int value = pot_readings[pot_index][i];
        int count = 0;
        
        for (int j = 0; j < POT_READING_BUFFER_SIZE; ++ j) {
            if (pot_readings[pot_index][j] == value) {
                ++ count;
            }
        }

        if (count > most_popular_count) {
            most_popular_count = count;
            most_popular = value;
        }
    }

    return most_popular;
}

/*/
void muxRead()
{
    selectMuxChannel(mux_channel);

    // TODO: Figure out why analog pins aren't defined for mock Arduino
#if !defined(MOCK_ARDUINO)
    pot_readings[mux_channel][pot_reading_index] = analogRead(MUX_1_COM_PIN) >> 3;
    pot_readings[16 + mux_channel][pot_reading_index] = analogRead(MUX_2_COM_PIN) >> 3;
    pot_readings[32 + mux_channel][pot_reading_index] = analogRead(MUX_3_COM_PIN) >> 3;
    if (mux_channel < 8) {
        pot_readings[48 + mux_channel][pot_reading_index] = analogRead(MAIN_MUX_COM_PIN) >> 3;
    }
#endif

    if (++ channel == 16) {
        ++ pot_reading_index;
        pot_reading_index %= POT_READING_BUFFER_SIZE;
    }
}
    */

void handleInput(bool warmup)
{
    unsigned long pot_read_time_start = millis();
    for (int mux_channel = 0; mux_channel < 16; ++ mux_channel) {
        selectMuxChannel(mux_channel);

        // TODO: Figure out why analog pins aren't defined for mock Arduino
#if !defined(MOCK_ARDUINO)
        pot_readings[mux_channel][pot_reading_index] = analogRead(MUX_1_COM_PIN) >> 3;
        pot_readings[16 + mux_channel][pot_reading_index] = analogRead(MUX_2_COM_PIN) >> 3;
        pot_readings[32 + mux_channel][pot_reading_index] = analogRead(MUX_3_COM_PIN) >> 3;
        if (mux_channel < 8) {
            pot_readings[48 + mux_channel][pot_reading_index] = analogRead(MAIN_MUX_COM_PIN) >> 3;
        }

        if (MIDI.read()) {
            last_midi_event_time = millis();
            top_bar.setMidiIndicatorState(true);
        }
#endif

        bool is_touched = touchscreen.touched();
        TS_Point point(0, 0, 0);
        
        if (is_touched) {
            point = touchscreen.getPoint();
#if !defined(MOCK_ARDUINO)
            point = TS_Point(319 - point.y, point.x, point.z);
#endif
        }
        ui.handleTouchInput(is_touched, point.x, point.y);
    }

#if WITH_DEBUG_PAGE == 1
    if (pager.isCurrentPage(debug_page)) {
        debug_page.updatePotReadTimeMeasurement(millis() - pot_read_time_start);
    }
#endif

    ++ pot_reading_index;
    pot_reading_index %= POT_READING_BUFFER_SIZE;

    for (int i = 0; i < 56; ++ i) {
        uint8_t value = getMostCommonPotReading(i);
        if (value != previous_pot_readings[i]) {
            previous_pot_readings[i] = value;
#if WITH_DEBUG_PAGE == 1
            if (pager.isCurrentPage(debug_page)) {
                debug_page.updateControlValue(i / 16, i % 16, value);
            }
#endif
        }
    }

    if ((last_midi_event_time > 0) && (millis() - last_midi_event_time >= MIDI_INDICATOR_BLINK_TIME)) {
        top_bar.setMidiIndicatorState(false);
        last_midi_event_time = 0;
    }
}

void firstTimeInit()
{
    settings.signature = EEPROM_SIGNATURE;
    settings.midi_channel = 1;

#if !defined(MOCK_ARDUINO)
    EEPROM.put(EEPROM_SETTINGS_OFFSET, settings);
#endif
}

void setup()
{
    pinMode(MUX_S0_PIN, OUTPUT);
    pinMode(MUX_S1_PIN, OUTPUT);
    pinMode(MUX_S2_PIN, OUTPUT);
    pinMode(MUX_S3_PIN, OUTPUT);

    // TODO: Would be nice to work out why analog pins aren't defined for mock Arduino
#if !defined(MOCK_ARDUINO)
    pinMode(MAIN_MUX_COM_PIN, INPUT);
    pinMode(MUX_1_COM_PIN, INPUT);
    pinMode(MUX_2_COM_PIN, INPUT);
    pinMode(MUX_3_COM_PIN, INPUT);
#endif

    pinMode(DISPLAY_BACKLIGHT_PIN, OUTPUT);
    digitalWrite(DISPLAY_BACKLIGHT_PIN, LOW);

    ui.setGraphicsTable(graphics);

    screen.begin();
    screen.setRotation(1);

#if WITH_ASSERTS == 1
    SetDebugScreen(&screen);
#endif

    ui.begin(BACKGROUND_COLOUR);

    Splash splash(ui);

    splash.show();
    digitalWrite(DISPLAY_BACKLIGHT_PIN, HIGH);

#if !defined(MOCK_ARDUINO)
    EEPROM.get(EEPROM_SETTINGS_OFFSET, settings);
    if (settings.signature != EEPROM_SIGNATURE)
        firstTimeInit();

    if (!touchscreen.begin(40, &Wire)) {
        // TODO
    }

    MIDI.begin(MIDI_CHANNEL_OMNI);
    MIDI.setThruFilterMode(midi::Thru::SameChannel);
#endif

    // Only take initial pot readings
    for (int i = 0; i < POT_READING_BUFFER_SIZE; ++ i) {
        handleInput(true);
    }

    delay(1000);
    
    digitalWrite(DISPLAY_BACKLIGHT_PIN, LOW);
    splash.hide();

    top_bar.show();
    pager.setPage(page);
    digitalWrite(DISPLAY_BACKLIGHT_PIN, HIGH);
}

void loop()
{
    handleInput(false);
    ui.process();
}
