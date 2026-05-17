#ifndef PANEL_H
#define PANEL_H 1

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include "debug.h"

// Core graphic IDs used for actual drawing operations
enum {
    GRAPHIC_SET_COLOUR_OP = 0,
    GRAPHIC_PIXEL_OP,
    GRAPHIC_HORIZONTAL_LINE_OP,
    GRAPHIC_VERTICAL_LINE_OP,
    GRAPHIC_LINE_OP,
    GRAPHIC_RECTANGLE_OP,
    GRAPHIC_FILLED_RECTANGLE_OP,
    GRAPHIC_SQUARE_OP,
    GRAPHIC_FILLED_SQUARE_OP,
    GRAPHIC_SET_TEXT_SIZE_OP,
    GRAPHIC_CHARACTER_OP,
    GRAPHIC_BITMAP_OP,
    GRAPHIC_END_OP,

    GRAPHIC_FIRST_ID
};

// Macros for defining graphics
#define BEGIN_GRAPHIC(name)     const uint8_t PROGMEM name[] = {
#define END_GRAPHIC()           GRAPHIC_END_OP };
#define GRAPHIC(g, x, y)                        g, x, y,
#define GRAPHIC_SET_COLOUR(x)                   GRAPHIC_SET_COLOUR_OP, (x & 0xff), (x >> 8),
#define GRAPHIC_PIXEL(x, y)                     GRAPHIC_PIXEL_OP, x, y,
#define GRAPHIC_HORIZONTAL_LINE(y, x1, x2)      GRAPHIC_HORIZONTAL_LINE_OP, y, x1, x2,
#define GRAPHIC_VERTICAL_LINE(x, y1, y2)        GRAPHIC_VERTICAL_LINE_OP, x, y1, y2,
#define GRAPHIC_LINE(x1, y1, x2, y2)            GRAPHIC_LINE_OP, x1, y1, x2, y2,
#define GRAPHIC_RECTANGLE(x, y, w, h)           GRAPHIC_RECTANGLE_OP, x, y, w, h,
#define GRAPHIC_FILLED_RECTANGLE(x, y, w, h)    GRAPHIC_FILLED_RECTANGLE_OP, x, y, w, h,
#define GRAPHIC_SQUARE(x, y, size)              GRAPHIC_SQUARE_OP, x, y, size,
#define GRAPHIC_FILLED_SQUARE(x, y, size)       GRAPHIC_FILLED_SQUARE_OP, x, y, size,
#define GRAPHIC_BITMAP(x, y, width, height, ...)  GRAPHIC_BITMAP_OP, x, y, width, height, __VA_ARGS__,
#define GRAPHIC_SET_TEXT_SIZE(size)             GRAPHIC_SET_TEXT_SIZE_OP, size,
#define GRAPHIC_CHARACTER(x, y, c)              GRAPHIC_CHARACTER_OP, x, y, c,

#define COLOUR_BLACK                0x0000
#define COLOUR_WHITE                0xffff
#define COLOUR_GREY                 0x7bef
#define COLOUR_DARK_GREY            0x39e7
#define COLOUR_BRIGHT_RED           0xf800
#define COLOUR_RED                  0x7800
#define COLOUR_DARK_RED             0x4800
#define COLOUR_BRIGHT_GREEN         0x07e0
#define COLOUR_GREEN                0x03e0
#define COLOUR_DARK_GREEN           0x01e0
#define COLOUR_BRIGHT_BLUE          0x001f
#define COLOUR_BLUE                 0x0014
#define COLOUR_DARK_BLUE            0x000c
#define COLOUR_YELLOW               0xffe0

enum TouchEventType {
    TouchStartEvent,
    TouchMoveEvent,
    TouchEndEvent,
    TouchEnterEvent,        // Touch moved into a hotspot
    TouchLeaveEvent,        // Touch left a hotspot
    TouchTapEvent,          // Touch start and end occurred quickly in the same hotspot
    TouchHoldEvent          // Touch start and end occurred in same hotspot with some delay
};

struct Hotspot {
    bool operator==(const Hotspot &other)
    {
        return id == other.id;
    }

    bool operator!=(const Hotspot &other)
    {
        return id != other.id;
    }

    uint8_t id;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
};

class Panel;

class UI {
    friend class Panel;

    public:
        UI(Adafruit_GFX &screen)
        : m_screen(screen), m_first_panel(NULL), m_is_touched(false),
          m_touch_start_time(0), m_last_touch_x(-1), m_last_touch_y(-1),
          m_initial_touch_panel(NULL), m_last_touch_panel(NULL), m_same_hotspot(false)
        {
        }

        void begin(uint16_t colour)
        {
            m_background_colour = colour;
            m_screen.fillScreen(colour);
        }


        void setGraphicsTable(const uint8_t * const table[])
        {
            ASSERT(m_graphics_table);
            m_graphics_table = table;
        }

        const uint8_t * const getGraphic(uint8_t graphic_id) const
        {
            ASSERT(graphic_id >= GRAPHIC_FIRST_ID);
            return (const uint8_t *)pgm_read_ptr(&(m_graphics_table[graphic_id - GRAPHIC_FIRST_ID]));
        }

        int16_t width() const
        {
            return m_screen.width();
        }

        int16_t height() const
        {
            return m_screen.height();
        }

        Panel *getPanelAt(int16_t x, int16_t y) const;

        void handleTouchInput(bool touched, int16_t x, int16_t y);

    private:
        Adafruit_GFX &m_screen;
        Panel *m_first_panel;
        const uint8_t* const* m_graphics_table;
        uint16_t m_background_colour;
        bool m_is_touched;
        unsigned long m_touch_start_time;
        int16_t m_last_touch_x;
        int16_t m_last_touch_y;
        Panel *m_initial_touch_panel;
        Hotspot m_initial_touch_hotspot;
        Panel *m_last_touch_panel;
        Hotspot m_last_touch_hotspot;
        bool m_same_hotspot;
};



#define s_gfx (&(m_ui.m_screen))

class Panel: public Print {
    friend class UI;

    public:
        Panel(UI &ui, int16_t x, int16_t y, int16_t width, int16_t height)
        : m_ui(ui), m_x(x), m_y(y), m_width(width), m_height(height), m_colour(0xffff),
          m_force_background_colour(false), m_visible(false), m_number_of_hotspots(0),
          m_hotspots(NULL)
        {
            m_next_panel = ui.m_first_panel;
            ui.m_first_panel = this;
        }

        virtual ~Panel()
        {
            if (m_ui.m_first_panel == this) {
                m_ui.m_first_panel = m_next_panel;
            } else {
                Panel *panel = m_ui.m_first_panel;
                while (panel->m_next_panel != this) {
                    ASSERT(panel->m_next_panel);
                    panel = panel->m_next_panel;
                }
                panel->m_next_panel = m_next_panel;
            }
        }

        void show()
        {
            m_visible = true;
            draw();
        }

        void hide()
        {
            m_force_background_colour = true;
            s_gfx->setTextColor(m_ui.m_background_colour);
            m_visible = false;
            draw();
            m_force_background_colour = false;
            s_gfx->setTextColor(m_colour);
        }

        Hotspot getHotspotAt(uint16_t x, uint16_t y) const
        {            
            Hotspot hotspot;

            ASSERT(m_number_of_hotspots == 0 || m_hotspots);
            for (int i = 0; i < m_number_of_hotspots; ++ i) {
                hotspot.x = pgm_read_word(&m_hotspots[i].x);
                if (x < hotspot.x) continue;
                hotspot.width = pgm_read_word(&m_hotspots[i].width);
                if (x >= hotspot.x + hotspot.width) continue;
                hotspot.y = pgm_read_word(&m_hotspots[i].y);
                if (y < hotspot.y) continue;
                hotspot.height = pgm_read_word(&m_hotspots[i].height);
                if (y >= hotspot.y + hotspot.height) continue;
                hotspot.id = pgm_read_byte(&m_hotspots[i].id);
                return hotspot;
            }

            // Default hotspot spans the entire screen
            hotspot.id = 255;
            hotspot.x = 0;
            hotspot.y = 0;
            hotspot.width = m_width;
            hotspot.height = m_height;

            return hotspot;
        }

    protected:
        virtual void draw() = 0;

        void fill(uint16_t colour)
        {
            s_gfx->fillRect(m_x, m_y, m_width, m_height, m_force_background_colour ? m_ui.m_background_colour : colour);
        }

        void drawPixel(int16_t x, int16_t y, uint16_t colour)
        {
            s_gfx->drawPixel(m_x + x, m_y + y, m_force_background_colour ? m_ui.m_background_colour : colour);
        }
        
        void drawPixel(int16_t x, int16_t y)
        {
            drawPixel(x, y, getActiveColour());
        }

        void setColour(uint16_t colour)
        {
            m_colour = colour;
            if (!m_force_background_colour) {
                s_gfx->setTextColor(colour);
            }
        }
        
        void forceBackgroundColour(bool force = true)
        {
            m_force_background_colour = force;
            s_gfx->setTextColor(force ? m_ui.m_background_colour : m_colour);
        }

        void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
        {
            s_gfx->drawLine(m_x + x1, m_y + y1, m_x + x2, m_y + y2, getActiveColour());
        }

        void fillRectangle(int16_t x, int16_t y, int16_t width, int16_t height)
        {
            s_gfx->fillRect(m_x + x, m_y + y, width, height, getActiveColour());
        }

        void drawRectangle(int16_t x, int16_t y, int16_t width, int16_t height)
        {
            s_gfx->drawRect(m_x + x, m_y + y, width, height, getActiveColour());
        }

        /* TODO: Other shapes */

        void drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[],
                        int16_t width, int16_t height)
        {
            s_gfx->drawBitmap(m_x + x, m_y + y, bitmap, width, height, getActiveColour());
        }

        void setTextSize(uint8_t s)
        {
            s_gfx->setTextSize(s);
        }

        void setCursor(int16_t x, int16_t y)
        {
            s_gfx->setCursor(m_x + x, m_y + y);
        }

        void drawText(int16_t x, int16_t y, const char *string)
        {
            int16_t previous_x = s_gfx->getCursorX(),
                    previous_y = s_gfx->getCursorY();
            s_gfx->setCursor(m_x + x, m_y + y);
            print(string);
            s_gfx->setCursor(previous_x, previous_y);
        }

        using Print::write;

        virtual size_t write(uint8_t c)
        {
            return s_gfx->write(c);
        }

        void drawGraphic(uint8_t graphic_id, uint16_t x, uint16_t y)
        {
            // TODO: Save also cursor, text size?
            uint16_t original_colour = m_colour;
            doDrawGraphic(graphic_id, x, y);
            m_colour = original_colour;
        }

        void setHotspots(uint16_t count, const Hotspot hotspots[])
        {
            m_number_of_hotspots = count;
            m_hotspots = hotspots;
        }

        virtual void onTouchEvent(TouchEventType type, uint8_t hotspot_id, int16_t x, int16_t y) = 0;

    private:
        uint16_t getActiveColour()
        {
            return m_force_background_colour ? m_ui.m_background_colour : m_colour;
        }

        void doDrawGraphic(uint8_t graphic_id, uint16_t x, uint16_t y)
        {
            uint8_t data[4] = {0, 0, 0, 0};
            int i = 0;
            uint8_t data_count = 0;
            const uint8_t *graphic_data = m_ui.getGraphic(graphic_id);
            if (!graphic_data) return;

            uint8_t op = pgm_read_byte(&(graphic_data[i ++]));
            while (op != GRAPHIC_END_OP) {
                switch (op) {
                    case GRAPHIC_END_OP:
                        data_count = 0;
                        break;

                    case GRAPHIC_SET_TEXT_SIZE_OP:
                        data_count = 1;
                        break;

                    case GRAPHIC_PIXEL_OP:
                    case GRAPHIC_SET_COLOUR_OP:
                        data_count = 2;
                        break;

                    case GRAPHIC_HORIZONTAL_LINE_OP:
                    case GRAPHIC_VERTICAL_LINE_OP:
                    case GRAPHIC_SQUARE_OP:
                    case GRAPHIC_FILLED_SQUARE_OP:
                    case GRAPHIC_CHARACTER_OP:
                        data_count = 3;
                        break;

                    case GRAPHIC_LINE_OP:
                    case GRAPHIC_RECTANGLE_OP:
                    case GRAPHIC_FILLED_RECTANGLE_OP:
                    case GRAPHIC_BITMAP_OP:
                        data_count = 4;
                        break;

                    default:
                        // Reference to another graphic
                        data_count = 2;
                        break;
                };

                for (int j = 0; j < data_count; ++ j) {
                    data[j] = pgm_read_byte(&(graphic_data[i ++]));
                }

                switch (op) {
                    case GRAPHIC_END_OP:
                        break;

                    case GRAPHIC_SET_COLOUR_OP:
                        if (!m_force_background_colour) {
                            uint16_t colour;
                            memcpy(&colour, data, sizeof(uint16_t));
                            setColour(colour);
                        }
                        break;

                    case GRAPHIC_PIXEL_OP:
                        drawPixel(x + data[0], y + data[1]);
                        break;

                    case GRAPHIC_LINE_OP:
                        drawLine(x + data[0], y + data[1], x + data[2], y + data[3]);
                        break;

                    case GRAPHIC_HORIZONTAL_LINE_OP:
                        drawLine(x + data[1], y + data[0], x + data[2], y + data[0]);
                        break;

                    case GRAPHIC_VERTICAL_LINE_OP:
                        drawLine(x + data[0], y + data[1], x + data[0], y + data[2]);
                        break;

                    case GRAPHIC_RECTANGLE_OP:
                        drawRectangle(x + data[0], y + data[1], data[2], data[3]);
                        break;

                    case GRAPHIC_FILLED_RECTANGLE_OP:
                        fillRectangle(x + data[0], y + data[1], data[2], data[3]);
                        break;

                    case GRAPHIC_SQUARE_OP:
                        drawRectangle(x + data[0], y + data[1], data[2], data[2]);
                        break;

                    case GRAPHIC_FILLED_SQUARE_OP:
                        fillRectangle(x + data[0], y + data[1], data[2], data[2]);
                        break;

                    case GRAPHIC_SET_TEXT_SIZE_OP:
                        setTextSize(data[0]);
                        break;

                    case GRAPHIC_CHARACTER_OP:
                        setCursor(x + data[0], y + data[1]);
                        print((char)data[2]);
                        break;

                    case GRAPHIC_BITMAP_OP:
                        drawBitmap(x + data[0], y + data[1], &(graphic_data[i]), data[2], data[3]);
                        i += ((data[2] + 7) / 8) * data[3];
                        break;

                    default:
                        // Nested graphic
                        doDrawGraphic(op, x + data[0], y + data[1]);
                        break;
                };

                op = pgm_read_byte(&(graphic_data[i ++]));
            }
        }

        Panel *m_next_panel;
        UI &m_ui;
        int16_t m_x;
        int16_t m_y;
        int16_t m_width;
        int16_t m_height;
        uint16_t m_colour;
        bool m_force_background_colour;
        bool m_visible;
        int16_t m_number_of_hotspots;
        const Hotspot *m_hotspots;
};

#endif
