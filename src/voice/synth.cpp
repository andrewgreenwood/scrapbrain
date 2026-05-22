#include "notarduino.h"
#include "synth.h"

Synth::Synth(uint8_t max_polyphony)
: m_max_polyphony(max_polyphony), m_last_process_time(millis())
{
    m_notes = new NoteMapping[max_polyphony];
    for (int i = 0; i < m_max_polyphony; ++ i) {
        m_notes[i].channel = 0;
        m_notes[i].note_number = 0;
        m_notes[i].on = false;
        m_notes[i].sustain = false;
        m_notes[i].release_time_remaining = 0;
        m_notes[i].note = NULL;
    }

    for (int i = 0; i < 16; ++ i) {
        m_channels[i].pitch_bend = 8192;
        m_channels[i].sustaining = false;
    }
}

void Synth::process()
{
    // Max time (in ms) for an unsigned 32-bit integer is around 49 days
    // The 'on_duration' for a note will also overflow around this time
    // Hopefully nobody will want a MIDI note to play for this long!

    // TODO: Handle overflow of millis()
    uint32_t now = millis();
    uint32_t time_delta = now - m_last_process_time;

    for (int i = 0; i < m_max_polyphony; ++ i) {
        NoteMapping &note_mapping = m_notes[i];

        if (note_mapping.on)
            note_mapping.on_duration += time_delta;

        if (note_mapping.note) {
            note_mapping.note->update(note_mapping.on_duration);

            if ((!note_mapping.on) && (!note_mapping.sustain)) {
                // Free up any notes after their release time
                if (note_mapping.release_time_remaining > 0) {
                    if (note_mapping.release_time_remaining <= time_delta) {
                        note_mapping.release_time_remaining = 0;
                    } else {
                        note_mapping.release_time_remaining -= time_delta;
                    }
                } else {
                    terminateNote(note_mapping);
                }
            }
        }
    }

    m_last_process_time = now;
}

void Synth::noteOn(uint8_t channel, uint8_t note, uint8_t velocity)
{
    if ((!isValidChannel(channel)) || (!isValidNote(note)))
        return;

    // Restart the note if already playing
    if (isNoteOn(channel, note)) {
        noteOff(channel, note);
    }

    NoteMapping *note_mapping = NULL;

    // Look for an unused note mapping
    for (int i = 0; i < m_max_polyphony; ++ i) {
        if (!m_notes[i].note) {
            note_mapping = &m_notes[i];
            break;
        }
    }

    if (!note_mapping) {
        // Try to steal the oldest released note on this channel
        uint16_t shortest_release_time = 65535;
        for (int i = 0; i < m_max_polyphony; ++ i) {
            if ((m_notes[i].note) && (!m_notes[i].on) && (m_notes[i].channel == channel)) {
                if (m_notes[i].release_time_remaining < shortest_release_time) {
                    note_mapping = &m_notes[i];
                    shortest_release_time = note_mapping->release_time_remaining;
                }
            }
        }
    }

    if (!note_mapping) {
        // Try to steal the oldest released note on any channel
        uint16_t shortest_release_time = 65535;
        for (int i = 0; i < m_max_polyphony; ++ i) {
            if ((m_notes[i].note) && (!m_notes[i].on)) {
                if (m_notes[i].release_time_remaining < shortest_release_time) {
                    note_mapping = &m_notes[i];
                    shortest_release_time = note_mapping->release_time_remaining;
                }
            }
        }
    }

    if (!note_mapping) {
        // Try to steal the longest-playing note on this channel
        uint16_t longest_note_duration = 0;
        for (int i = 0; i < m_max_polyphony; ++ i) {
            if ((m_notes[i].note) && (m_notes[i].on) && (m_notes[i].channel == channel) && (m_notes[i].on_duration > longest_note_duration)) {
                note_mapping = &m_notes[i];
                longest_note_duration = m_notes[i].on_duration;
            }
        }
    }

    if (!note_mapping) {
        // If this channel has no playing notes, steal a note from any channel
        uint16_t longest_note_duration = 0;
        for (int i = 0; i < m_max_polyphony; ++ i) {
            if ((m_notes[i].note) && (m_notes[i].on) && (m_notes[i].on_duration > longest_note_duration)) {
                note_mapping = &m_notes[i];
                longest_note_duration = m_notes[i].on_duration;
            }
        }
    }

    // Must have a note mapping by now, so if we don't have one that's an error
    if (!note_mapping)
        return;

    if (note_mapping->note)
        terminateNote(*note_mapping);

    SynthNote *note_object = allocateNote(channel);
    if (!note)
        return;

    note_mapping->channel = channel;
    note_mapping->note_number = note;
    note_mapping->note = note_object;
    note_mapping->on = true;
    note_mapping->on_duration = 0;

    note_object->setPitch(note, m_channels[channel].pitch_bend);

    note_object->on(velocity);
    note_object->update(0);
}

void Synth::noteOff(uint8_t channel, uint8_t note)
{
    if ((!isValidChannel(channel)) || (!isValidNote(note)) || (!isNoteOn(channel, note)))
        return;


    NoteMapping *note_mapping = getNoteMapping(channel, note);
    if (!note_mapping)
        return;

    note_mapping->on = false;
    note_mapping->sustain = m_channels[channel].sustaining;

    if (!note_mapping->sustain) {
        note_mapping->release_time_remaining = note_mapping->note->off();
    }
}

void Synth::controlChange(uint8_t channel, uint8_t control, uint8_t value)
{
    if (!isValidChannel(channel))
        return;

    int16_t adjusted_value = setControl(channel, control, value);

    switch (control) {
        case 64:    // Sustain pedal
            setSustain(channel, value >= 64);
            break;

        case 120:   // All sound off
            stopAllNotes(channel, true);
            break;

        case 122:
            stopAllNotes(channel, false);
            break;
    }

    for (int i = 0; i < m_max_polyphony; ++ i) {
        if ((m_notes[i].note) && (m_notes[i].channel == channel)) {
            m_notes[i].note->setControl(control, adjusted_value);
        }
    }
}

void Synth::pitchBend(uint8_t channel, uint16_t amount)
{
    if (!isValidChannel(channel))
        return;

    m_channels[channel].pitch_bend = amount;

    for (int i = 0; i < m_max_polyphony; ++ i) {
        if ((m_notes[i].note) && (m_notes[i].channel == channel)) {
            m_notes[i].note->setPitch(m_notes[i].note_number, m_channels[channel].pitch_bend);
        }
    }
}

void Synth::sustain(uint8_t channel, bool state)
{
    controlChange(channel, 64, state ? 127 : 0);
}

void Synth::silence(uint8_t channel, bool immediate)
{
    controlChange(channel, immediate ? 120 : 122, 0);
}

int16_t Synth::scaleControlValue(uint8_t value, int16_t min, int16_t max)
{
    return min + (((uint8_t)value * ((max - min) + 1)) / 128);
}

void Synth::setSustain(uint8_t channel, bool state)
{
    bool stop_notes = ((m_channels[channel].sustaining) && (!state));

    m_channels[channel].sustaining = state;

    if (stop_notes) {
        for (int i = 0; i < m_max_polyphony; ++ i) {
            NoteMapping &note_mapping = m_notes[i];
            if ((note_mapping.note) && (note_mapping.channel == channel) && (note_mapping.sustain)) {
                note_mapping.release_time_remaining = note_mapping.note->off();
                note_mapping.sustain = false;
            }
        }
    }
}

void Synth::stopAllNotes(uint8_t channel, bool immediate)
{
    for (int i = 0; i < m_max_polyphony; ++ i) {
        NoteMapping &note_mapping = m_notes[i];
        if ((note_mapping.note) && (note_mapping.channel == channel)) {
            note_mapping.on = false;

            if (!immediate) {
                note_mapping.sustain = m_channels[channel].sustaining;
                if (!note_mapping.sustain) {
                    note_mapping.release_time_remaining = note_mapping.note->off();
                }
            } else {
                terminateNote(note_mapping);
            }
        }
    }
}
