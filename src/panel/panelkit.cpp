#include "panelkit.h"

Panel *UI::getPanelAt(int16_t x, int16_t y) const
{
    for (Panel *panel = m_first_panel; panel; panel = panel->m_next_panel) {
        //printf("%p\n", panel);
        if (!panel->m_visible) continue;
        if (x < panel->m_x) continue;
        if (y < panel->m_y) continue;
        if (x >= panel->m_x + panel->m_width) continue;
        if (y >= panel->m_y + panel->m_height) continue;
        //printf("Touching %p\n", panel);
        //Hotspot hotspot = panel->getHotspotAt(x, y);
        //printf("%d\n", hotspot.id);
        return panel;
    }
    return NULL;
}


void UI::handleTouchInput(bool touched, int16_t x, int16_t y)
{
    Panel *panel = NULL;
    TouchEventType event_type;

    // Figure out if this is a touch start/move/end and at what coordinates
    if (touched) {
        if (!m_is_touched) {
            event_type = TouchStartEvent;
            m_is_touched = true;
            m_touch_start_time = millis();
        } else if ((x != m_last_touch_x) || (y != m_last_touch_y)) {
            event_type = TouchMoveEvent;
        } else {
            return;
        }
    } else if (m_is_touched) {
        event_type = TouchEndEvent;
        m_is_touched = false;
        // Use previous X/Y for end of a touch event
        x = m_last_touch_x;
        y = m_last_touch_y;
    } else {
        return;
    }

    panel = getPanelAt(x, y);
    if (panel) {
        Hotspot hotspot = panel->getHotspotAt(x - panel->m_x, y - panel->m_y);

        panel->onTouchEvent(event_type, hotspot.id, x - (panel->m_x + hotspot.x), y - (panel->m_y + hotspot.y));

        switch (event_type) {
            case TouchStartEvent:
                m_initial_touch_panel = m_last_touch_panel = panel;
                m_initial_touch_hotspot = m_last_touch_hotspot = hotspot;
                m_same_hotspot = true;
                break;

            case TouchMoveEvent:
                if ((panel != m_last_touch_panel) || (hotspot != m_last_touch_hotspot)) {
                    m_same_hotspot = false;
                    if (m_last_touch_panel) {
                        m_last_touch_panel->onTouchEvent(TouchLeaveEvent, m_last_touch_hotspot.id,
                                                        m_last_touch_x - (m_last_touch_panel->m_x + m_last_touch_hotspot.x),
                                                        m_last_touch_y - (m_last_touch_panel->m_y + m_last_touch_hotspot.y));
                    }
                    panel->onTouchEvent(TouchEnterEvent, hotspot.id,
                                        x - (panel->m_x + hotspot.x),
                                        y - (panel->m_y + hotspot.y));
                    m_last_touch_panel = panel;
                    m_last_touch_hotspot = hotspot;
                }
                break;

            case TouchEndEvent:
                if (m_same_hotspot) {
                    if (millis() - m_touch_start_time < 500) {
                        panel->onTouchEvent(TouchTapEvent, hotspot.id, x - (panel->m_x + hotspot.x), y - (panel->m_y + hotspot.y));
                    } else {
                        // TODO: Long press
                    }
                }
                break;
        }
    }

    m_last_touch_x = x;
    m_last_touch_y = y;
}
