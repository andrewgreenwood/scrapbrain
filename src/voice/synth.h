#ifndef SYNTH_H
#define SYNTH_H 1

#ifdef ARDUINO
    #include <arduino.h>
#else
    #include <cstdint>
#endif

class SynthNote {
    public:
        virtual ~SynthNote()
        { }

        // Called before a note starts, and in response to pitch bends
        virtual void setPitch(uint8_t note, int16_t bend_amount)
        { }

        // Called when a note is to start playing
        virtual void on(uint8_t velocity)
        { }

        // Called when a note is to stop playing
        // This should return the release time (in milliseconds) before the
        // note is to end
        virtual uint16_t off()
        { return 0; }

        // Called when a note ends (reaches the end of the release time
        // returned by off)
        virtual void end()
        { }

        // Called for each note in response to control changes
        virtual void setControl(uint8_t control, int16_t value)
        { }

        // Called with the elapsed time since note on, until note end
        // (it will conntinue through note off)
        virtual void update(uint32_t elapsed)
        { }
};

class Synth {
    public:
        Synth(uint8_t max_polyphony);

        virtual ~Synth()
        { }

        void process();

        // This does not set control values (synth implementation must do this
        // e.g. when allocating a note.)
        void noteOn(uint8_t channel, uint8_t note, uint8_t velocity);

        void noteOff(uint8_t channel, uint8_t note);

        // The synth implementation and its notes will always be called, even
        // if a control change is automatically handled (e.g. sustain)
        void controlChange(uint8_t channel, uint8_t control, uint8_t value);

        void pitchBend(uint8_t channel, uint16_t amount);

        void sustain(uint8_t channel, bool state);

        void silence(uint8_t channel, bool immediate = false);

        static int16_t scaleControlValue(uint8_t value, int16_t min, int16_t max);

    protected:
        // Called when a note is being allocated for a channel
        // Channel is provided so the note can be initialised (e.g. with control values)
        // or in case some other channel-specific behaviour is desired
        virtual SynthNote* allocateNote(uint8_t channel)
        { return NULL; }

        // Called when a note is being freed
        virtual void freeNote(SynthNote *note)
        { }

        // Called when a control change occurs for a channel, so that the
        // synth implementation may store the new value for subsequent
        // note allocations
        virtual int16_t setControl(uint8_t channel, uint8_t control, uint8_t value)
        { }

    private:
        struct {
            uint16_t pitch_bend : 14;
            uint16_t sustaining : 1;
        } m_channels[16];

        struct NoteMapping {
            uint8_t channel     : 4;
            uint8_t on          : 1;            // Note on/off state
            uint8_t sustain     : 1;            // True whilst sustaining after note off
            uint8_t note_number : 7;
            uint32_t on_duration;               // Time (ms) since note on
            uint16_t release_time_remaining;    // Time (ms) after note off before note is freed
            SynthNote *note;
        } *m_notes;

        // NOTE: There may be multiple note mappings for a single note number, as some
        // notes may be in the release phase!
        NoteMapping *getNoteMapping(uint8_t channel, uint8_t note) const
        {
            for (int i = 0; i < m_max_polyphony; ++ i) {
                NoteMapping &note_mapping = m_notes[i];
                if ((note_mapping.note) && (note_mapping.channel == channel) && (note_mapping.note_number == note) && (note_mapping.on))
                    return &m_notes[i];
            }

            return NULL;
        }

        void terminateNote(NoteMapping &note_mapping)
        {
            note_mapping.sustain = false;
            note_mapping.note->off();
            note_mapping.release_time_remaining = 0;
            note_mapping.note->end();
            freeNote(note_mapping.note);
            note_mapping.note = NULL;
        }

        bool isValidChannel(uint8_t channel) const
        {
            return channel < 16;
        }

        bool isValidNote(uint8_t note) const
        {
            return note < 128;
        }

        bool isNoteOn(uint8_t channel, uint8_t note) const
        {
            return getNoteMapping(channel, note);
        }

        void setSustain(uint8_t channel, bool state);

        void stopAllNotes(uint8_t channel, bool immediate);

        uint8_t m_max_polyphony;
        unsigned long m_last_process_time;
};

#endif
