#include <Arduino.h>

#ifdef MOCK_ARDUINO
#include "mockscreen.h"
#else
#include "Adafruit_ILI9341.h"
#include "Adafruit_FT6206.h"
#endif

#include "panelkit.h"

#ifdef MOCK_ARDUINO
MockScreen screen(240, 320);
MockTouchScreen touchscreen;
#else
Adafruit_ILI9341 screen(3, 2);
Adafruit_FT6206 touchscreen;
#endif

#define BACKGROUND_COLOUR           0x1082
#define TOPBAR_COLOUR               0x0000
#define CARRIER_OPERATOR_COLOUR     0x2589
#define MODULATOR_OPERATOR_COLOUR   0xf3e4
#define OPERATOR_LINK_COLOUR        0xffff

#define INDICATOR_OUTLINE_COLOUR    0xffff
#define GREEN_INDICATOR_COLOUR      0x2589
#define GREEN_INDICATOR_LOW_COLOUR  0x0000

#define BUTTON_OUTLINE_COLOUR       0x0841

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



class TopBar: public Panel {
    public:
        TopBar(UI &ui)
        : Panel(ui, 0, 0, 320, 24)
        {
        }

        virtual ~TopBar()
        {
        }

        void setIndicatorState(int8_t indicator, bool state)
        {
            int16_t x[3] = { 151, 202, 263 };

            if ((indicator < 0) || (indicator > 2)) {
                return;
            }

            if (state) {
                drawGraphic(GRAPHIC_GREEN_INDICATOR, x[indicator], 9);
            } else {
                setColour(BACKGROUND_COLOUR);
                fillRectangle(x[indicator], 9, 5, 5);
            }
        }

        virtual void onTouchEvent(TouchEventType type, uint8_t hotspot_id, int16_t x, int16_t y)
        {
        }

    private:
        virtual void draw()
        {
            fill(TOPBAR_COLOUR);
            setTextSize(1);
            setColour(COLOUR_WHITE);

            setCursor(8, 8);
            print("Scrap Brain YM2612");

            drawGraphic(GRAPHIC_INDICATOR_OUTLINE, 150, 8);
            setCursor(163, 8);
            print("MIDI");

            drawGraphic(GRAPHIC_INDICATOR_OUTLINE, 201, 8);
            setCursor(214, 8);
            print("Gate 1");

            drawGraphic(GRAPHIC_INDICATOR_OUTLINE, 262, 8);
            setCursor(275, 8);
            print("Gate 2");
        }
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
          m_pager(pager),  m_number_of_hotspots(0), m_hotspots(NULL)
        { }

        virtual void draw()
        {
            setCursor(0, 0);
            setColour(COLOUR_BRIGHT_GREEN);
            print((uintptr_t)this);
        }

//        virtual void onTouchEvent(TouchEventType type, uint8_t hotspot_id, int16_t x, int16_t y)
//        {
//            printf("Page %p event type %d hotspot %d x %d y %d\n", this, type, hotspot_id, x, y);
//        }

    protected:
        virtual void setHotspots(uint16_t count, const Hotspot hotspots[])
        {
            m_number_of_hotspots = count;
            m_hotspots = hotspots;
        }

    private:
        Pager &m_pager;
        int16_t m_number_of_hotspots;
        const Hotspot *m_hotspots;
};

void Pager::setPage(Page &page)
{
    if (m_current_page) {
        m_current_page->hide();
    }
    m_current_page = &page;
    page.Panel::setHotspots(page.m_number_of_hotspots, page.m_hotspots);
    m_current_page->show();
}




#define PAGE_HEADER_Y   11


enum {
    PatchOptionsPageBackButtonHotspotId = 1,
    NumberOfPatchOptionsPageHotspots
};

class PatchOptionsPage: public Page {
    public:
        PatchOptionsPage(Pager &pager)
        : Page(pager)
        {
            setHotspots(NumberOfPatchOptionsPageHotspots, s_hotspots);
        }

        virtual void draw()
        {
            setColour(COLOUR_WHITE);
            drawGraphic(GRAPHIC_SMALL_BUTTON_OUTLINE, 20, PAGE_HEADER_Y);
            drawGraphic(GRAPHIC_LEFT_CHEVRON, 28, PAGE_HEADER_Y + 6);
            setTextSize(2);
            drawText(65, PAGE_HEADER_Y + 5, "Patch");

            setColour(COLOUR_DARK_RED);
            drawRectangle(0,   0,  70, 50);
            drawRectangle(45,  70, 50, 75);
            drawRectangle(105, 70, 50, 75);
            drawRectangle(165, 70, 50, 75);
            drawRectangle(225, 70, 50, 75);

    //GRAPHIC_UP_ARROW_IMAGE,
//    GRAPHIC_DOWN_ARROW_IMAGE,

            setTextSize(1);
            setColour(COLOUR_WHITE);
            drawText(55, 120, "Reset");
            setColour(COLOUR_YELLOW);
            drawGraphic(GRAPHIC_RESET_ARROW_IMAGE, 58, 82);

            setColour(COLOUR_WHITE);
            drawText(109, 120, "Factory");
            drawText(109, 133, "Patches");
            drawGraphic(GRAPHIC_UP_ARROW_IMAGE, 125, 76);
            setColour(COLOUR_YELLOW);
            drawGraphic(GRAPHIC_BOX_IMAGE, 113, 85);

            setColour(COLOUR_WHITE);
            drawText(178, 120, "Load");
            drawGraphic(GRAPHIC_UP_ARROW_IMAGE, 185, 76);
            setColour(COLOUR_YELLOW);
            drawGraphic(GRAPHIC_OPEN_FOLDER_IMAGE, 175, 87);

            setColour(COLOUR_WHITE);
            drawText(238, 120, "Save");
            drawGraphic(GRAPHIC_DOWN_ARROW_IMAGE, 245, 76);
            setColour(COLOUR_YELLOW);
            drawGraphic(GRAPHIC_OPEN_FOLDER_IMAGE, 235, 87);
        }

        virtual void onTouchEvent(TouchEventType type, uint8_t hotspot_id, int16_t x, int16_t y);

    private:
        static const Hotspot PROGMEM s_hotspots[NumberOfPatchOptionsPageHotspots];
};

const Hotspot PROGMEM PatchOptionsPage::s_hotspots[NumberOfPatchOptionsPageHotspots] = {
    { .id = PatchOptionsPageBackButtonHotspotId,         .x = 0,   .y = 0,  .width = 70, .height = 50  }
};


enum {
    MidiChannelBackButtonHotspotId = 1,
    PrimaryMidiChannelDecrementHotspotId,
    PrimaryMidiChannelIncrementHotspotId,
    SecondaryMidiChannelDecrementHotspotId,
    SecondaryMidiChannelIncrementHotspotId,
    NumberOfMidiChannelPageHotspots
};

class SettingsPage: public Page {
    public:
        SettingsPage(Pager &pager)
        : Page(pager), m_primary_channel(0), m_secondary_channel(1)
        {
            setHotspots(NumberOfMidiChannelPageHotspots, s_hotspots);
            // TODO: Read from EEPROM
        }

        virtual void draw()
        {
            setColour(COLOUR_WHITE);
            drawGraphic(GRAPHIC_SMALL_BUTTON_OUTLINE, 20, PAGE_HEADER_Y);
            drawGraphic(GRAPHIC_LEFT_CHEVRON, 28, PAGE_HEADER_Y + 6);
            setTextSize(2);
            drawText(65, PAGE_HEADER_Y + 5, "MIDI Channels");

            setTextSize(1);
            drawText(70, 76, "Primary");
            drawText(200, 76, "Secondary");
            drawGraphic(GRAPHIC_MIDI_CONNECTOR, 75, 98);
            drawGraphic(GRAPHIC_MIDI_CONNECTOR, 210, 98);
            setTextSize(2);

            drawPrimaryChannel();
            drawGraphic(GRAPHIC_LEFT_CHEVRON, 50, 108);
            drawGraphic(GRAPHIC_RIGHT_CHEVRON, 124, 108);

            drawSecondaryChannel();
            drawGraphic(GRAPHIC_LEFT_CHEVRON, 185, 108);
            drawGraphic(GRAPHIC_RIGHT_CHEVRON, 259, 108);

            // Hotspots
            setColour(COLOUR_DARK_RED);
            drawRectangle(0,   0,  70, 50);
            drawRectangle(30,  90, 50, 50);
            drawRectangle(102, 90, 50, 50);
            drawRectangle(165, 90, 50, 50);
            drawRectangle(237, 90, 50, 50);
        }

        void drawPrimaryChannel()
        {
            setColour(COLOUR_WHITE);
            setCursor(80, 146);
            if (m_primary_channel < 9) print("0");
            print(m_primary_channel + 1);
        }

        void drawSecondaryChannel()
        {
            setColour(COLOUR_WHITE);
            setCursor(215, 146);
            if (m_secondary_channel < 9) print("0");
            print(m_secondary_channel + 1);
        }
        
        virtual void onTouchEvent(TouchEventType type, uint8_t hotspot_id, int16_t x, int16_t y);

    private:
        uint8_t m_primary_channel;
        uint8_t m_secondary_channel;
        static const Hotspot PROGMEM s_hotspots[NumberOfMidiChannelPageHotspots];
};

const Hotspot PROGMEM SettingsPage::s_hotspots[NumberOfMidiChannelPageHotspots] = {
    { .id = MidiChannelBackButtonHotspotId,         .x = 0,   .y = 0,  .width = 70, .height = 50  },
    { .id = PrimaryMidiChannelDecrementHotspotId,   .x = 30,  .y = 90, .width = 50, .height = 50 },
    { .id = PrimaryMidiChannelIncrementHotspotId,   .x = 102, .y = 90, .width = 50, .height = 50 },
    { .id = SecondaryMidiChannelDecrementHotspotId, .x = 165, .y = 90, .width = 50, .height = 50 },
    { .id = SecondaryMidiChannelIncrementHotspotId, .x = 237, .y = 90, .width = 50, .height = 50 }
};


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
    NumberOfMainPageHotspots
};

class MainPage: public Page {
    public:
        MainPage(Pager &pager)
        : Page(pager), m_algorithm(0)
        {
            setHotspots(NumberOfMainPageHotspots, s_hotspots);
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
            drawGraphic(GRAPHIC_COG_IMAGE, 206, 166);

            setTextSize(1);
            setColour(COLOUR_WHITE);
            drawText(87, 195, "Patches");
            drawText(193, 195, "Settings");

            drawCurrentAlgorithm();
        }

        virtual void drawCurrentAlgorithm()
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

            drawGraphic(graphic_ids[m_algorithm], 85, 12);

            int selection_x = m_algorithm < 4 ? 18 : 264;
            int selection_y = 14 + ((m_algorithm % 4) * 48);

            drawGraphic(GRAPHIC_ALGORITHM_SELECT, selection_x, selection_y);
        }

        virtual void onTouchEvent(TouchEventType type, uint8_t hotspot_id, int16_t x, int16_t y);

        uint8_t m_algorithm;
        static const Hotspot PROGMEM s_hotspots[NumberOfMainPageHotspots];
};

const Hotspot PROGMEM MainPage::s_hotspots[NumberOfMainPageHotspots] = {
    { .id = Algorithm1HotspotId,     .x = 0,   .y = 9,   .width = 75, .height = 48 },
    { .id = Algorithm2HotspotId,     .x = 0,   .y = 57,  .width = 75, .height = 48 },
    { .id = Algorithm3HotspotId,     .x = 0,   .y = 105, .width = 75, .height = 48 },
    { .id = Algorithm4HotspotId,     .x = 0,   .y = 153, .width = 75, .height = 48 },
    { .id = Algorithm5HotspotId,     .x = 244, .y = 9,   .width = 75, .height = 48 },
    { .id = Algorithm6HotspotId,     .x = 244, .y = 57,  .width = 75, .height = 48 },
    { .id = Algorithm7HotspotId,     .x = 244, .y = 105, .width = 75, .height = 48 },
    { .id = Algorithm8HotspotId,     .x = 244, .y = 153, .width = 75, .height = 48 },
    { .id = PatchButtonHotspotId,    .x = 83,  .y = 156, .width = 48, .height = 60 },
    { .id = SettingsButtonHotspotId, .x = 193, .y = 156, .width = 48, .height = 60 }
};


class Splash: public Panel {
    public:
        Splash(UI &ui)
        : Panel(ui, 0, 0, 320, 240)
        {
        }

        virtual void draw()
        {
            setTextSize(2);
            setCursor(100, 105);
            print("Scrap Brain");
            setTextSize(1);
            setCursor(100, 125);
            print("YM2612 Synthesiser");
        }

        virtual void onTouchEvent(TouchEventType type, uint8_t hotspot_id, int16_t x, int16_t y)
        {
        }
};


UI ui(screen);
TopBar topBar(ui);
Pager pager(ui, 0, 24, 320, 216);
MainPage page(pager);
PatchOptionsPage patch_options_page(pager);
SettingsPage settings_page(pager);



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
    if ((type == TouchTapEvent) && (hotspot_id == PatchOptionsPageBackButtonHotspotId)) {
        pager.setPage(page);
    }
}

void SettingsPage::onTouchEvent(TouchEventType type, uint8_t hotspot_id, int16_t x, int16_t y)
{
    //if (type == TouchStartEvent) {
    if ((type == TouchTapEvent) && (hotspot_id == MidiChannelBackButtonHotspotId)) {
        // TODO: Save changes to EEPROM
        pager.setPage(page);
    } else if (type == TouchStartEvent) {
        switch (hotspot_id) {
            case PrimaryMidiChannelDecrementHotspotId:
                if (m_primary_channel > 0) {
                    forceBackgroundColour(true);
                    drawPrimaryChannel();
                    forceBackgroundColour(false);
                    -- m_primary_channel;
                    drawPrimaryChannel();
                }
                break;

            case PrimaryMidiChannelIncrementHotspotId:
                if (m_primary_channel < 15) {
                    forceBackgroundColour(true);
                    drawPrimaryChannel();
                    forceBackgroundColour(false);
                    ++ m_primary_channel;
                    drawPrimaryChannel();
                }
                break;

            case SecondaryMidiChannelDecrementHotspotId:
                if (m_secondary_channel > 0) {
                    forceBackgroundColour(true);
                    drawSecondaryChannel();
                    forceBackgroundColour(false);
                    -- m_secondary_channel;
                    drawSecondaryChannel();
                }
                break;

            case SecondaryMidiChannelIncrementHotspotId:
                if (m_secondary_channel < 15) {
                    forceBackgroundColour(true);
                    drawSecondaryChannel();
                    forceBackgroundColour(false);
                    ++ m_secondary_channel;
                    drawSecondaryChannel();
                }
                break;
        };
    } else {
        //printf("%d\n", hotspot_id);
    }
}


void setup()
{
    pinMode(9, OUTPUT);
    digitalWrite(9, LOW);

    ui.setGraphicsTable(graphics);      // TODO
    //ui.setBackgroundColour(BACKGROUND_COLOUR);
    //setScreen(screen);
    screen.begin();
    screen.setRotation(3);
    //screen.fillScreen(BACKGROUND_COLOUR);
    //Panel::setScreen(screen);

    ui.begin(BACKGROUND_COLOUR);

    Splash splash(ui);

    splash.show();
    //digitalWrite(9, HIGH);

#if !defined(MOCK_ARDUINO)
    if (!touchscreen.begin(40, &Wire)) {
        // TODO
    }
#endif

    delay(1000);
    //pager.setPage(page);
    
    digitalWrite(9, LOW);
    splash.hide();

    topBar.show();
    pager.setPage(page);
    //digitalWrite(9, HIGH);
}

int ind = 0;

void loop()
{
    TS_Point point = touchscreen.getPoint();
#if !defined(MOCK_ARDUINO)
    point = TS_Point(point.y, 239 - point.x, point.z);
#endif
    ui.handleTouchInput(touchscreen.touched(), point.x, point.y);

//    pager.setPage(page);
//    topBar.setIndicatorState(ind, false);
//    delay(500);
//    pager.setPage(page2);
//    topBar.setIndicatorState(ind, true);
//    delay(500);

    ++ ind;
    ind %= 3;

/*
    topBar.show();
    pager.setPage(page);
    delay(500);
    topBar.hide();
    pager.setPage(page2);
    delay(500);
    */
}
