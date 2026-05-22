/*
    SCRAP BRAIN - YM2612 Hardware Synth - Panel
    Author: Andrew Greenwood
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
#include "graphics.h"
#include "common.h"
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

enum ControlIndex {
    // Mux A
    Op1_Sustain_ControlIndex = 0,
    Op1_Decay2_ControlIndex,
    Op1_Release_ControlIndex,
    Op1_AMStart_ControlIndex,
    Op1_Level_ControlIndex,
    Op1_Start_ControlIndex,
    Op1_Velocity_ControlIndex,
    Op1_EnvScale_ControlIndex,
    Op1_Detune_ControlIndex,
    Op1_Decay1_ControlIndex,
    Op1_FreqX_ControlIndex,
    Op1_Attack_ControlIndex,
    Op2_Attack_ControlIndex,
    Op2_Detune_ControlIndex,
    Op2_Decay1_ControlIndex,
    Op2_FreqX_ControlIndex,

    // Mux B
    Op2_Sustain_ControlIndex,
    Op2_Decay2_ControlIndex,
    Op2_Release_ControlIndex,
    Op2_AMStart_ControlIndex,
    Op2_Level_ControlIndex,
    Op2_Start_ControlIndex,
    Op2_Velocity_ControlIndex,
    Op2_EnvScale_ControlIndex,
    Op3_Sustain_ControlIndex,
    Op3_Decay2_ControlIndex,
    Op3_Release_ControlIndex,
    Op3_AMStart_ControlIndex,
    Op3_Level_ControlIndex,
    Op3_Start_ControlIndex,
    Op3_Velocity_ControlIndex,
    Op3_EnvScale_ControlIndex,

    // Mux C
    Op4_Sustain_ControlIndex,
    Op4_Decay2_ControlIndex,
    Op4_Release_ControlIndex,
    Op4_AMStart_ControlIndex,
    Op4_Level_ControlIndex,
    Op4_Start_ControlIndex,
    Op4_Velocity_ControlIndex,
    Op4_EnvScale_ControlIndex,
    Op3_Attack_ControlIndex,
    Op3_Detune_ControlIndex,
    Op3_Decay1_ControlIndex,
    Op3_FreqX_ControlIndex,
    Op4_FreqX_ControlIndex,
    Op4_Attack_ControlIndex,
    Op4_Detune_ControlIndex,
    Op4_Decay1_ControlIndex,

    // Mux D
    UNUSED1_ControlIndex,
    UNUSED2_ControlIndex,
    Op1_Feedback_ControlIndex,
    UNUSED3_ControlIndex,
    LFORate_ControlIndex,
    AMDepth_ControlIndex,
    PMStart_ControlIndex,
    PMDepth_ControlIndex,

    NumberOfPots,

    // Additional controls that aren't physical pots
    Algorithm_ControlIndex = NumberOfPots,

    NumberOfControls
};

const int8_t control_index_to_midi_controller_map[NumberOfControls] PROGMEM = {
    Op1_Sustain_MidiController,
    Op1_Decay2_MidiController,
    Op1_Release_MidiController,
    Op1_AMStart_MidiController,
    Op1_Level_MidiController,
    Op1_Start_MidiController,
    Op1_Velocity_MidiController,
    Op1_EnvScale_MidiController,
    Op1_Detune_MidiController,
    Op1_Decay1_MidiController,
    Op1_FreqX_MidiController,
    Op1_Attack_MidiController,
    Op2_Attack_MidiController,
    Op2_Detune_MidiController,
    Op2_Decay1_MidiController,
    Op2_FreqX_MidiController,
    Op2_Sustain_MidiController,
    Op2_Decay2_MidiController,
    Op2_Release_MidiController,
    Op2_AMStart_MidiController,
    Op2_Level_MidiController,
    Op2_Start_MidiController,
    Op2_Velocity_MidiController,
    Op2_EnvScale_MidiController,
    Op3_Sustain_MidiController,
    Op3_Decay2_MidiController,
    Op3_Release_MidiController,
    Op3_AMStart_MidiController,
    Op3_Level_MidiController,
    Op3_Start_MidiController,
    Op3_Velocity_MidiController,
    Op3_EnvScale_MidiController,
    Op4_Sustain_MidiController,
    Op4_Decay2_MidiController,
    Op4_Release_MidiController,
    Op4_AMStart_MidiController,
    Op4_Level_MidiController,
    Op4_Start_MidiController,
    Op4_Velocity_MidiController,
    Op4_EnvScale_MidiController,
    Op3_Attack_MidiController,
    Op3_Detune_MidiController,
    Op3_Decay1_MidiController,
    Op3_FreqX_MidiController,
    Op4_FreqX_MidiController,
    Op4_Attack_MidiController,
    Op4_Detune_MidiController,
    Op4_Decay1_MidiController,
    -1,
    -1,
    Op1_Feedback_MidiController,
    -1,
    LFORate_MidiController,
    AMDepth_MidiController,
    PMStart_MidiController,
    PMDepth_MidiController,
    Algorithm_MidiController
};

#define CONTROL_CASE(name)      case name##_MidiController: return name##_ControlIndex;
#define OP_CONTROL_CASE(name)   CONTROL_CASE(Op1_##name) \
                                CONTROL_CASE(Op2_##name) \
                                CONTROL_CASE(Op3_##name) \
                                CONTROL_CASE(Op4_##name)

int8_t midiControllerToControlIndex(int8_t midi_controller)
{
    switch (midi_controller) {
        CONTROL_CASE(LFORate)
        CONTROL_CASE(Algorithm)
        CONTROL_CASE(AMDepth)
        CONTROL_CASE(PMDepth)
        OP_CONTROL_CASE(AMStart)
        OP_CONTROL_CASE(Level)
        OP_CONTROL_CASE(Velocity)
        OP_CONTROL_CASE(Start)
        OP_CONTROL_CASE(FreqX)
        OP_CONTROL_CASE(Detune)
        CONTROL_CASE(Op1_Feedback)
        CONTROL_CASE(PMStart)
        OP_CONTROL_CASE(Attack)
        OP_CONTROL_CASE(Decay1)
        OP_CONTROL_CASE(Sustain)
        OP_CONTROL_CASE(Decay2)
        OP_CONTROL_CASE(Release)
        OP_CONTROL_CASE(EnvScale)
        default:    return -1;
    };
}

#undef CONTROL_CASE
#undef OP_CONTROL_CASE

int8_t controlIndexToMidiController(int8_t control_index)
{
    if ((control_index < 0) || (control_index >= NumberOfControls))
        return -1;

    return (int8_t)pgm_read_byte(&control_index_to_midi_controller_map[control_index]);
}

// Most recently loaded or saved patch (-1 for none)
#define NO_PATCH    -1
int8_t selected_patch = NO_PATCH;

// TODO: Pack
struct Patch {
    uint8_t controls[NumberOfControls];
    uint8_t reserved[64 - NumberOfControls];    // pad to 64 bytes length
};

// Current control values (loaded from patch, overridden by MIDI or panel)
uint8_t effective_control_values[NumberOfControls];

// Number of mux channels is (3x16)+8 == 56 (some are unused)
#define POT_READING_BUFFER_SIZE     4
uint8_t pot_readings[NumberOfPots][POT_READING_BUFFER_SIZE];
uint8_t previous_pot_readings[NumberOfPots];
uint8_t pot_reading_index = 0;

void updateDebugControlDisplay(uint8_t control_index);

// Send current value for a control
void sendControlValue(uint8_t control_index)
{
    int8_t midi_controller = controlIndexToMidiController(control_index);
    ASSERT(midi_controller != -1);
#if !defined(MOCK_ARDUINO)
    MIDI.sendControlChange(midi_controller, effective_control_values[control_index], settings.midi_channel + 1);
#endif
}

// Update the current value for a control and send it
void setControlValue(uint8_t control_index, uint8_t value)
{
    ASSERT(control_index < NumberOfControls);
    ASSERT(value < 0x80);

    if (effective_control_values[control_index] == value) return;

    effective_control_values[control_index] = value;
    sendControlValue(control_index);

#if WITH_DEBUG_PAGE == 1
    updateDebugControlDisplay(control_index);
#endif
}

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

// Re-reads all pots (does not affect algorithm)
void resetControls()
{
    // Each read is preceded by a dummy read to try to improve stability
    for (int i = 0; i < POT_READING_BUFFER_SIZE; ++ i) {
        for (int mux_channel = 0; mux_channel < 16; ++ mux_channel) {
            selectMuxChannel(mux_channel);
#if !defined(MOCK_ARDUINO)
            analogRead(MUX_1_COM_PIN);
            pot_readings[mux_channel][i] = analogRead(MUX_1_COM_PIN) >> 2;
            analogRead(MUX_2_COM_PIN);
            pot_readings[16 + mux_channel][i] = analogRead(MUX_2_COM_PIN) >> 2;
            analogRead(MUX_3_COM_PIN);
            pot_readings[32 + mux_channel][i] = analogRead(MUX_3_COM_PIN) >> 2;
            if (mux_channel < 8) {
                analogRead(MAIN_MUX_COM_PIN);
                pot_readings[48 + mux_channel][i] = analogRead(MAIN_MUX_COM_PIN) >> 2;
            }
#endif
        }
    }

    // Commit the pot readings
    for (int i = 0; i < NumberOfPots; ++ i) {
        uint8_t value = getMostCommonPotReading(i);
        previous_pot_readings[i] = value;
        setControlValue(i, value >> 1);
    }
}



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
          m_is_active(false), m_number_of_hotspots(0), m_hotspots(NULL) { }

        virtual ~Page() { }

        bool isActive()
        { return m_is_active; }

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

        bool m_is_active;
        int16_t m_number_of_hotspots;
        const Hotspot *m_hotspots;
};

void Pager::setPage(Page &page)
{
    if (m_current_page) {
        m_current_page->hide();
        m_current_page->onLeave();
        m_current_page->m_is_active = false;
    }
    m_current_page = &page;
    page.Panel::setHotspots(page.m_number_of_hotspots, page.m_hotspots);
    m_current_page->m_is_active = true;
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

// TODO: Move these
#define MIN_PATCH_INDEX             0
#define MAX_PATCH_INDEX             19

#define MIN_USER_PATCH_INDEX        0
#define MAX_USER_PATCH_INDEX        9

#define MIN_FACTORY_PATCH_INDEX     10
#define MAX_FACTORY_PATCH_INDEX     19

#define PAGE_HEADER_Y   11

class PatchSelectPage: public Page {
    public:
        enum Mode {
            Factory,
            Load,
            Save
        };

        PatchSelectPage(Pager &pager)
        : Page(pager), m_mode(Factory), m_patch(selected_patch), m_animation_start_time(0),
          m_animation_frame(0)
        {
            setHotspots(NumberOfHotspots, s_hotspots);
        }

        virtual ~PatchSelectPage() { }

        // Must be called before showing the page
        void setMode(Mode mode)
        {
            m_mode = mode;            
        }

    private:
        virtual void draw()
        {
            const __FlashStringHelper *titles[3] = {
                F("Factory Patches"),
                F("Load Patch"),
                F("Save Patch")
            };

            setColour(COLOUR_WHITE);
            drawGraphic(GRAPHIC_SMALL_BUTTON_OUTLINE, 20, PAGE_HEADER_Y);
            drawGraphic(GRAPHIC_LEFT_CHEVRON, 28, PAGE_HEADER_Y + 6);
            setTextSize(2);

            drawText(65, PAGE_HEADER_Y + 5, titles[m_mode]);

            setTextSize(1);
            setColour(COLOUR_BRIGHT_RED);
            drawText(25, 190, F("not implemented // todo"));

            drawPatches();
        }

        // Change which patch button is highlighted
        void setPatch(int8_t patch_index)
        {
            ASSERT((patch_index >= MIN_PATCH_INDEX) && (patch_index <= MAX_PATCH_INDEX));

            if (patch_index != m_patch) {
                m_patch = patch_index;
                if (isActive()) drawPatches();
            }
        }

        // Flash the currently selected patch button
        void flashCurrentPatchButton()
        {
            m_animation_start_time = millis();
            m_animation_frame = 0;
        }

        void drawPatchButton(int8_t button_index, uint16_t colour)
        {
            char label[3] = {0, 0, 0};
            int x, y;

            if ((button_index < 0) || (button_index > 9)) return;

            setTextSize(2);
            setColour(colour);

            if (button_index < 5) {
                x = 25 + (button_index * 57);
                y = 79;
                label[0] = '0';
                label[1] = '1' + button_index;
            } else {
                x = 25 + ((button_index - 5) * 57);
                y = 139;
                label[0] = button_index == 9 ? '1' : '0';
                label[1] = button_index == 9 ? '0' : '1' + button_index;
            }

            drawRectangle(x, y, 42, 42);
            drawText(x + 10, y + 13, label);
        }

        void drawPatches()
        {
            for (int button_index = 0; button_index < 10; ++ button_index) {
                int8_t patch_index = getPatchIndexForButton(button_index);
                uint16_t colour;
                if (isVacantPatchSlot(patch_index)) {
                    colour = COLOUR_GREY;
                } else if (m_patch == patch_index) {
                    colour = COLOUR_BRIGHT_GREEN;
                } else {
                    colour = COLOUR_WHITE;
                }
                drawPatchButton(button_index, colour);
            }
        }

        // Only user patch slots can be vacant
        bool isVacantPatchSlot(int8_t patch_index)
        {
            ASSERT((patch_index >= -10) && (patch_index <= 10) && (patch_index != 0));
            if (m_mode == Factory) {
                return false;
            } else {
#if !defined(MOCK_ARDUINO)
                return EEPROM.read(EEPROM_PATCHES_OFFSET + ((patch_index - 1) * sizeof(Patch))) == 0xff;
#else
                // For testing
                return patch_index > 4;
#endif
            }
        }

        // Using the current mode, get the patch index for a button
        int8_t getPatchIndexForButton(int8_t button_index)
        {
            ASSERT((button_index >= 0) && (button_index <= 9));
            if (m_mode == Factory) {
                return MIN_FACTORY_PATCH_INDEX + button_index;
            } else {
                return MIN_USER_PATCH_INDEX + button_index;
            }
        }

        // Determine which button coresponds to a given patch index
        int8_t getButtonIndexForPatch(int8_t patch_index)
        {
            if (patch_index == NO_PATCH) {
                return NO_PATCH;
            } else if ((patch_index >= MIN_FACTORY_PATCH_INDEX) && (patch_index <= MAX_FACTORY_PATCH_INDEX)) {
                return patch_index - MIN_FACTORY_PATCH_INDEX;
            } else if ((patch_index >= MIN_USER_PATCH_INDEX) && (patch_index <= MAX_USER_PATCH_INDEX)) {
                return patch_index - MIN_USER_PATCH_INDEX;
            }
            ASSERT(false);
            return NO_PATCH;
        }

        virtual void onEnter() { }
        virtual void onLeave() { }

        virtual void onTouchEvent(TouchEventType type, uint8_t hotspot_id, int16_t x, int16_t y);

        virtual void process();

        Mode m_mode;
        int8_t &m_patch;
        unsigned long m_animation_start_time;
        uint8_t m_animation_frame;

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
            MIDI.setInputChannel(settings.midi_channel + 1);
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

        void setAlgorithm(uint8_t algorithm)
        {
            if ((algorithm < 8) && (algorithm != m_algorithm)) {
                forceBackgroundColour(true);
                drawCurrentAlgorithm();
                m_algorithm = algorithm;
                forceBackgroundColour(false);
                drawCurrentAlgorithm();
            }
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
                setControlValue(Algorithm_ControlIndex, m_algorithm << 4);
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
    if (type == TouchTapEvent) {
        switch (hotspot_id) {
            case BackButtonHotspotId:
                pager.setPage(page);
                break;

            case ResetButtonHotspotId:
                resetControls();
                selected_patch = NO_PATCH;
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

void PatchSelectPage::process()
{
    if (m_animation_start_time > 0) {
        unsigned long elapsed = millis() - m_animation_start_time;
        uint8_t frame = elapsed / 100;

        if (frame != m_animation_frame) {
            int8_t button_index = getButtonIndexForPatch(m_patch);
            drawPatchButton(button_index, frame % 2 ? COLOUR_GREEN : COLOUR_BRIGHT_GREEN);
            m_animation_frame = frame;
        }

        // Stop animating
        if (frame > 5) {
            m_animation_start_time = 0;
        }
    }
}

void PatchSelectPage::onTouchEvent(TouchEventType type, uint8_t hotspot_id, int16_t x, int16_t y)
{
    // Ignore touch events whilst animating (indicates patch load/save)
    if (m_animation_start_time > 0) return;

    if (type == TouchTapEvent) {
        switch (hotspot_id) {
            case BackButtonHotspotId:
                pager.setPage(patch_options_page);
                break;

            default:
                if ((hotspot_id >= Patch1ButtonHotspotId) && (hotspot_id <= Patch10ButtonHotspotId)) {
                    int8_t patch_index = hotspot_id - Patch1ButtonHotspotId;
                    if ((m_mode != Load) || (!isVacantPatchSlot(patch_index))) {
                        setPatch(patch_index);
                        flashCurrentPatchButton();
                    }
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
    } else if (hotspot_id == NoteTriggerButtonId) {
# if !defined(MOCK_ARDUINO)
        if (type == TouchStartEvent) {
            MIDI.sendNoteOn(35, 127, settings.midi_channel + 1);
        } else if (type == TouchEndEvent) {
            MIDI.sendNoteOff(35, 127, settings.midi_channel + 1);
        }
#endif
    }
}
#endif

#define MIDI_INDICATOR_BLINK_TIME   250
unsigned long last_midi_event_time = 0;

void processMidi()
{
    int8_t control_index;

#if !defined(MOCK_ARDUINO)
    if (MIDI.read()) {
        last_midi_event_time = millis();
        top_bar.setMidiIndicatorState(true);

        if (MIDI.getType() == midi::ControlChange) {
            control_index = midiControllerToControlIndex(MIDI.getData1());
            if (control_index != -1) {
                effective_control_values[control_index] = MIDI.getData2();
                if (control_index == Algorithm_ControlIndex) {
                    page.setAlgorithm(MIDI.getData2() >> 4);
                }
#if WITH_DEBUG_PAGE == 1
                if ((pager.isCurrentPage(debug_page)) && (control_index < NumberOfPots)) {
                    debug_page.updateControlValue(control_index / 16, control_index % 16, MIDI.getData2());
                }
#endif
            }
        }
    }
#endif
}

#if WITH_DEBUG_PAGE == 1
void updateDebugControlDisplay(uint8_t control_index)
{
    if ((control_index < NumberOfPots) && (pager.isCurrentPage(debug_page))) {
        debug_page.updateControlValue(control_index / 16, control_index % 16, effective_control_values[control_index]);
    }
}
#endif

void processTouchscreenInput()
{
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

void firstTimeInit()
{
    settings.signature = EEPROM_SIGNATURE;
    settings.midi_channel = 0;

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
    digitalWrite(DISPLAY_BACKLIGHT_PIN, HIGH);

    splash.show();
#ifdef MOCK_ARDUINO
    // Ensures the mock Arduino screen displays the splash
    UiProcessEvents();
#endif

#if !defined(MOCK_ARDUINO)
    EEPROM.get(EEPROM_SETTINGS_OFFSET, settings);
    if (settings.signature != EEPROM_SIGNATURE)
        firstTimeInit();

    if (!touchscreen.begin(40, &Wire)) {
        // TODO
    }

    MIDI.begin(settings.midi_channel + 1);
    MIDI.setThruFilterMode(midi::Thru::SameChannel);
#endif

    // Keep the splash page up briefly to allow the synth some time to be ready
    delay(1000);

    resetControls();

    // Select first algorithm
    setControlValue(Algorithm_ControlIndex, 0);

    delay(1000);

    digitalWrite(DISPLAY_BACKLIGHT_PIN, LOW);
    splash.hide();

    top_bar.show();
    pager.setPage(page);
    digitalWrite(DISPLAY_BACKLIGHT_PIN, HIGH);
}

void loop()
{
    unsigned long pot_read_time_start = millis();

    for (int mux_channel = 0; mux_channel < 16; ++ mux_channel) {
        selectMuxChannel(mux_channel);

        // TODO: Figure out why analog pins aren't defined for mock Arduino
#if !defined(MOCK_ARDUINO)
        // Each read is preceded by a dummy read to try to improve stability
        analogRead(MUX_1_COM_PIN);
        pot_readings[mux_channel][pot_reading_index] = analogRead(MUX_1_COM_PIN) >> 2;
        processMidi();

        analogRead(MUX_2_COM_PIN);
        pot_readings[16 + mux_channel][pot_reading_index] = analogRead(MUX_2_COM_PIN) >> 2;
        processMidi();

        analogRead(MUX_3_COM_PIN);
        pot_readings[32 + mux_channel][pot_reading_index] = analogRead(MUX_3_COM_PIN) >> 2;
        processMidi();

        if (mux_channel < 8) {
            analogRead(MAIN_MUX_COM_PIN);
            pot_readings[48 + mux_channel][pot_reading_index] = analogRead(MAIN_MUX_COM_PIN) >> 2;
        }
        processMidi();

#endif
        processTouchscreenInput();
    }

#if WITH_DEBUG_PAGE == 1
    if (pager.isCurrentPage(debug_page)) {
        debug_page.updatePotReadTimeMeasurement(millis() - pot_read_time_start);
    }
#endif

    ++ pot_reading_index;
    pot_reading_index %= POT_READING_BUFFER_SIZE;

    for (int i = 0; i < NumberOfPots; ++ i) {
        uint8_t value = getMostCommonPotReading(i);
        if (abs((int16_t)value - (int16_t)previous_pot_readings[i]) > 1) {
            previous_pot_readings[i] = value;
            setControlValue(i, value >> 1);
        }
    }

    if ((last_midi_event_time > 0) && (millis() - last_midi_event_time >= MIDI_INDICATOR_BLINK_TIME)) {
        top_bar.setMidiIndicatorState(false);
        last_midi_event_time = 0;
    }

    ui.process();
}
