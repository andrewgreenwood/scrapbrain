/*
    SCRAP BRAIN - YM2612 Hardware Synth - Panel
    Author: Andrew Greenwood

    All output goes to both L/R channels
*/

#include <MIDI.h>
#include "common.h"
#include "synth.h"

// TODO: IC pin? (currently just pulled HIGH)
#define YM2612_CLOCK_PIN    3
#define YM2612_CS_PIN       A1
#define YM2612_WR_PIN       A2
#define YM2612_RD_PIN       A3
#define YM2612_A0_PIN       A4
#define YM2612_A1_PIN       A5

// YM2612 data pins are split between PORTB and PORTD
#define YM2612_DATA_PORTB_BITMASK   0x0b
#define YM2612_DATA_PORTD_BITMASK   (~YM2612_DATA_PORTB_BITMASK)

float noteToPitch(float note)
{
    int offset = note - 72;
    return (616 * pow(2, (float)offset / 12));
}

unsigned short noteToYM2612Frequency(float note)
{
    unsigned char block = 0;
    unsigned short frequency;
    float offset = note - 92;

    if (offset >= 1) {
        block = offset / 12;
        offset -= 12 * block;
    }

    block += 4;

    frequency = (978 * pow(2, offset / 12));

    return ((unsigned short)block << 11) | frequency;
}

namespace YM2612 {
    struct Controls {
        Controls()
        : algorithm(0), op1_feedback(0), left_output_enable(1), right_output_enable(1),
          am_sensitivity(0), pm_sensitivity(0), pm_start(0)
        {
            for (int op = 0; op < 4; ++ op) {
                operators[op].start = 0;
                operators[op].frequency_multiplier = 1;
                operators[op].detune = 0;
                operators[op].attack_rate = 31;
                operators[op].rate_scaling = 0;
                operators[op].decay1_rate = 0;
                operators[op].am_start = 0;
                operators[op].decay2_rate = 0;
                operators[op].release_rate = 15;
                operators[op].sustain_level = 0;
                operators[op].ssgeg_on = false;
                operators[op].ssgeg = 0;
                operators[op].velocity_to_level = 127;
                operators[op].level = 127;
            }
        };

        uint8_t algorithm               : 3;
        uint8_t op1_feedback            : 3;
        uint8_t left_output_enable      : 1;
        uint8_t right_output_enable     : 1;
        uint8_t am_sensitivity          : 2;
        uint8_t pm_sensitivity          : 3;
        uint8_t pm_start                : 7;

        struct {
            uint8_t start                   : 7;
            uint8_t frequency_multiplier    : 4;
            uint8_t detune                  : 3;
            uint8_t attack_rate             : 5;
            uint8_t rate_scaling            : 2;
            uint8_t decay1_rate             : 5;
            uint8_t am_start                : 7;
            uint8_t decay2_rate             : 5;
            uint8_t release_rate            : 4;
            uint8_t sustain_level           : 4;
            uint8_t ssgeg_on                : 1;
            uint8_t ssgeg                   : 3;
            uint8_t velocity_to_level       : 7;
            uint8_t level                   : 7;
        } operators[4];
    };

    enum GlobalRegister {
        LFO_GlobalRegister = 0x22,
        TimerAFrequencyHigh_GlobalRegister = 0x24,
        TimerAFrequencyLow_GlobalRegister = 0x25,
        TimerBFrequency_GlobalRegister = 0x26,
        Channel3ModeAndTimer_GlobalRegister = 0x27,
        Key_GlobalRegister = 0x28,
        DACOutput_GlobalRegister = 0x2a,
        DACEnable_GlobalRegister = 0x2b
    };

    enum ChannelRegister {
        Frequency_ChannelRegister = 0xa0,
        ALGO_FEED_ChannelRegister = 0xb0,
        L_R_AMS_PMS_ChannelRegister = 0xb4
    };

    static uint8_t pack_ALGO_FEED_Value(uint8_t algo, uint8_t feed)
    { return (algo & 0x7) | ((feed & 0x7) << 3); }

    static uint8_t pack_L_R_AMS_PMS_Value(uint8_t l, uint8_t r, uint8_t ams, uint8_t pms)
    { return (((l & 0x1) << 7) | ((r & 0x1) << 6) | ((ams & 0x3) << 4) | ((pms) & 0x7)); }

    enum OperatorRegister {
        MUL_DT_OperatorRegister = 0x30,
        TL_OperatorRegister = 0x40,
        AR_RS_OperatorRegister = 0x50,
        DR_AMON_OperatorRegister = 0x60,
        SR_OperatorRegister = 0x70,
        RR_SL_OperatorRegister = 0x80,
        SSGEG_OperatorRegister = 0x90,
    };

    static uint8_t pack_MUL_DT_Value(uint8_t mul, uint8_t dt)
    { return (mul & 0x0f) | ((dt & 0x7) << 4); }

    static uint8_t pack_AR_RS_Value(uint8_t ar, uint8_t rs)
    { return (ar & 0x1f) | ((rs & 0x3) << 6); }

    static uint8_t pack_DR_AMON_Value(uint8_t dr, bool amon)
    { return (dr & 0x1f) | (amon << 7); }

    static uint8_t pack_RR_SL_Value(uint8_t rr, uint8_t sl)
    { return (rr & 0xf) | ((sl & 0xf) << 4); }

    // TODO: Test
    /*
    uint8_t read()
    {
        uint8_t data;

        digitalWrite(YM2612_CS_PIN, LOW);
        delayMicroseconds(1);

        DDRB &= ~YM2612_DATA_PORTB_BITMASK;
        DDRD &= ~YM2612_DATA_PORTD_BITMASK;

        digitalWrite(YM2612_A0_PIN, LOW);
        digitalWrite(YM2612_A1_PIN, LOW);

        delayMicroseconds(1);
        digitalWrite(YM2612_RD_PIN, LOW);
        delayMicroseconds(5);

        data  = PORTB & YM2612_DATA_PORTB_BITMASK;
        data |= PORTD & YM2612_DATA_PORTD_BITMASK;

        digitalWrite(YM2612_RD_PIN, HIGH);
        delayMicroseconds(5);

        digitalWrite(YM2612_CS_PIN, HIGH);

        return data;
    }
    */

    // Wait for YM2612 busy bit to be clear
    // (doesn't work currently)
    void wait()
    {
        //while (Read() & 0x80) { }
    }

    void write(uint8_t address, uint8_t data)
    {
        wait();

        digitalWrite(YM2612_CS_PIN, LOW);
        delayMicroseconds(1);

        digitalWrite(YM2612_A0_PIN, address & 2 ? HIGH : LOW);
        digitalWrite(YM2612_A1_PIN, address & 1 ? HIGH : LOW);

        DDRB |= YM2612_DATA_PORTB_BITMASK;
        DDRD |= YM2612_DATA_PORTD_BITMASK;

        PORTB = (PORTB & ~YM2612_DATA_PORTB_BITMASK) | (data & YM2612_DATA_PORTB_BITMASK);
        PORTD = (PORTD & ~YM2612_DATA_PORTD_BITMASK) | (data & YM2612_DATA_PORTD_BITMASK);

        delayMicroseconds(1);
        digitalWrite(YM2612_WR_PIN, LOW);
        delayMicroseconds(5);

        digitalWrite(YM2612_WR_PIN, HIGH);
        delayMicroseconds(5);

        digitalWrite(YM2612_CS_PIN, HIGH);
    }

    void setGlobalRegister(GlobalRegister reg, uint8_t value)
    {
        write(0, reg);
        write(2, value);
    }

    void init()
    {
        DDRB &= ~YM2612_DATA_PORTB_BITMASK;
        DDRD &= ~YM2612_DATA_PORTD_BITMASK;

        pinMode(YM2612_CS_PIN, OUTPUT);
        pinMode(YM2612_RD_PIN, OUTPUT);
        pinMode(YM2612_WR_PIN, OUTPUT);
        pinMode(YM2612_A0_PIN, OUTPUT);
        pinMode(YM2612_A1_PIN, OUTPUT);

        digitalWrite(YM2612_CS_PIN, HIGH);
        digitalWrite(YM2612_RD_PIN, HIGH);
        digitalWrite(YM2612_WR_PIN, HIGH);
        digitalWrite(YM2612_A0_PIN, LOW);
        digitalWrite(YM2612_A1_PIN, LOW);

        // Output 8MHz PWM for clock
        pinMode(YM2612_CLOCK_PIN, OUTPUT);
        TCCR2A = bit(COM2B0) | bit(WGM21);
        TCCR2B = bit(CS20);
        TCNT2 = 0;
        OCR2B = 0;

        // Initialise (TODO)
        //pinMode(A0, OUTPUT);
        //digitalWrite(A0, LOW);
        //delay(10);
        //digitalWrite(A0, HIGH);
        //delay(10);

        // Reset and stop timers, set channel 3 to normal mode
        setGlobalRegister(Channel3ModeAndTimer_GlobalRegister, 0x00);
        setGlobalRegister(TimerAFrequencyHigh_GlobalRegister, 0x00);
        setGlobalRegister(TimerAFrequencyLow_GlobalRegister, 0x00);
        setGlobalRegister(TimerBFrequency_GlobalRegister, 0x00);

        // LFO off
        setGlobalRegister(LFO_GlobalRegister, 0x00);

        // Note off (all channels)
        setGlobalRegister(Key_GlobalRegister, 0x00);
        setGlobalRegister(Key_GlobalRegister, 0x01);
        setGlobalRegister(Key_GlobalRegister, 0x02);
        setGlobalRegister(Key_GlobalRegister, 0x04);
        setGlobalRegister(Key_GlobalRegister, 0x05);
        setGlobalRegister(Key_GlobalRegister, 0x06);

        // Turn off DAC
        setGlobalRegister(Key_GlobalRegister, 0x00);
        setGlobalRegister(DACEnable_GlobalRegister, 0x00);
    }

    class Voice: public SynthNote {
        public:
            Voice()
            : m_controls(NULL), m_fm_channel(-1), m_velocity(0), m_pm_started(false)
            {
            }

            Voice(Controls &controls, int fm_channel);

            virtual ~Voice()
            { }

            virtual void setPitch(uint8_t note, int16_t bend_amount);

            virtual void on(uint8_t velocity);

            virtual uint16_t off();

            virtual void end()
            {
            }

            virtual void setControl(uint8_t control, int16_t value);

            virtual void update(uint32_t elapsed);

        private:
            void updateChannelRegister(uint8_t reg);

            void updateOperatorRegister(uint8_t op, uint8_t reg);

            void setChannelRegister(ChannelRegister reg, uint8_t value)
            {
                int base_address = m_fm_channel < 3 ? 0 : 1;
                write(base_address, reg + (m_fm_channel % 3));
                write(base_address + 2, value);
            }

            void setOperatorRegister(uint8_t op, OperatorRegister reg, uint8_t value)
            {
                static uint8_t reg_offset[4] = {0x0, 0x8, 0x4, 0xc};
                int base_address = m_fm_channel < 3 ? 0 : 1;
                write(base_address, reg + (m_fm_channel % 3) + reg_offset[op]);
                write(base_address + 2, value);
            }

            // Scales a level based on velocity and velocity sensitivity
            uint8_t getScaledLevel(uint8_t velocity, uint8_t velocity_sensitivity, uint8_t level)
            {
                // TODO: Re-enable this, for now just return level for testing purposes
                return level;
                uint16_t curved_velocity = ((uint16_t)velocity * velocity) / 127;
                uint16_t velocity_range = ((uint16_t)velocity_sensitivity * level) / 127;
                uint16_t base_level = level - velocity_range;
                return base_level + (curved_velocity * velocity_range) / 127;
            }

            Controls *m_controls;
            int m_fm_channel;
            uint8_t m_velocity;
            bool m_pm_started;
            struct {
                uint8_t started : 1;
                uint8_t am_started : 1;
            } m_timed_controls[4];
    };

    class Synth: public ::Synth {
        //friend class Voice;

        public:
            Synth()
            : ::Synth(6),
              m_allocated_voice_bitmap(0)
            {
                // Only class member initialisation should take place here
                // This will likely be called before setup() - anything that needs
                // to set registers should go into Init()

                for (int ch = 0; ch < 16; ++ ch) {
                    for (int op = 0; op < 4; ++ op) {
                        m_controls[ch].operators[op].start = 0;
                        m_controls[ch].operators[op].am_start = 0;  //10 * (4 - op);      // test
                    }
                }
            }

            virtual ~Synth()
            { }

            static void init();

            virtual SynthNote* allocateNote(uint8_t channel);

            virtual void freeNote(SynthNote *voice);

            virtual int16_t setControl(uint8_t channel, uint8_t control, uint8_t value);

        private:
            Voice m_voices[6];
            uint8_t m_allocated_voice_bitmap;
            Controls m_controls[16];
    };

    SynthNote* Synth::allocateNote(uint8_t channel)
    {
        for (int i = 0; i < 6; ++ i) {
            if (!(m_allocated_voice_bitmap & (1 << i))) {
                m_allocated_voice_bitmap |= 1 << i;
                m_voices[i] = Voice(m_controls[channel], i);
                return &m_voices[i];
            }
        }

        return NULL;
    }

    void Synth::freeNote(SynthNote *voice)
    {
        int i;
        for (i = 0; i < 6; ++ i) {
            if (&m_voices[i] == voice) {
                m_allocated_voice_bitmap &= ~(1 << i);
                break;
            }
        }
    }

    // This packs the value for the register that needs updating, whilst also saving
    int16_t Synth::setControl(uint8_t channel, uint8_t control, uint8_t value)
    {
        #define INVERT(v) \
            (127-v)

        #define CHANNEL_CONTROL(prop) \
            m_controls[channel].prop

        #define OP_CONTROL(op, prop) \ 
            m_controls[channel].operators[op].prop

        switch (control) {
            case LFORate_MidiController:
                setGlobalRegister(LFO_GlobalRegister, scaleControlValue(value, 7, 15));
                break;

            case Algorithm_MidiController:
                return pack_ALGO_FEED_Value(CHANNEL_CONTROL(algorithm) = value >> 4,
                                            CHANNEL_CONTROL(op1_feedback));
            case AMDepth_MidiController:
                return CHANNEL_CONTROL(am_sensitivity) = value >> 5;
            case PMDepth_MidiController:
                return CHANNEL_CONTROL(pm_sensitivity) = value >> 4;
            case Op1_AMStart_MidiController:
                return OP_CONTROL(0, am_start) = value;
            case Op2_AMStart_MidiController:
                return OP_CONTROL(1, am_start) = value;
            case Op3_AMStart_MidiController:
                return OP_CONTROL(2, am_start) = value;
            case Op4_AMStart_MidiController:
                return OP_CONTROL(3, am_start) = value;
            case Op1_Level_MidiController:
                return OP_CONTROL(0, level) = value;
            case Op2_Level_MidiController:
                return OP_CONTROL(1, level) = value;
            case Op3_Level_MidiController:
                return OP_CONTROL(2, level) = value;
            case Op4_Level_MidiController:
                return OP_CONTROL(3, level) = value;
            case Op1_Velocity_MidiController:
                return OP_CONTROL(0, velocity_to_level) = value;
            case Op2_Velocity_MidiController:
                return OP_CONTROL(1, velocity_to_level) = value;
            case Op3_Velocity_MidiController:
                return OP_CONTROL(2, velocity_to_level) = value;
            case Op4_Velocity_MidiController:
                return OP_CONTROL(3, velocity_to_level) = value;
            case Op1_Start_MidiController:
                return OP_CONTROL(0, start) = value;
            case Op2_Start_MidiController:
                return OP_CONTROL(1, start) = value;
            case Op3_Start_MidiController:
                return OP_CONTROL(2, start) = value;
            case Op4_Start_MidiController:
                return OP_CONTROL(3, start) = value;
            case Op1_FreqX_MidiController:
                return pack_MUL_DT_Value(OP_CONTROL(0, frequency_multiplier) = value >> 3,
                                         OP_CONTROL(0, detune));
            case Op2_FreqX_MidiController:
                return pack_MUL_DT_Value(OP_CONTROL(1, frequency_multiplier) = value >> 3,
                                         OP_CONTROL(1, detune));
            case Op3_FreqX_MidiController:
                return pack_MUL_DT_Value(OP_CONTROL(2, frequency_multiplier) = value >> 3,
                                         OP_CONTROL(2, detune));
            case Op4_FreqX_MidiController:
                return pack_MUL_DT_Value(OP_CONTROL(3, frequency_multiplier) = value >> 3,
                                         OP_CONTROL(3, detune));
            case Op1_Detune_MidiController:
                return pack_MUL_DT_Value(OP_CONTROL(0, frequency_multiplier),
                                         OP_CONTROL(0, detune) = (7 - scaleControlValue(value, 1, 7)));
            case Op2_Detune_MidiController:
                return pack_MUL_DT_Value(OP_CONTROL(1, frequency_multiplier),
                                         OP_CONTROL(1, detune) = (7 - scaleControlValue(value, 1, 7)));
            case Op3_Detune_MidiController:
                return pack_MUL_DT_Value(OP_CONTROL(2, frequency_multiplier),
                                         OP_CONTROL(2, detune) = (7 - scaleControlValue(value, 1, 7)));
            case Op4_Detune_MidiController:
                return pack_MUL_DT_Value(OP_CONTROL(3, frequency_multiplier),
                                         OP_CONTROL(3, detune) = (7 - scaleControlValue(value, 1, 7)));
            case Op1_Feedback_MidiController:
                // TODO: This is temporarily forced to zero due to panel not being wired up for it
                value = 0;                
                return pack_ALGO_FEED_Value(CHANNEL_CONTROL(algorithm),
                                            CHANNEL_CONTROL(op1_feedback) = value >> 4);
            case PMStart_MidiController:
                return CHANNEL_CONTROL(pm_start) = value;
            case Op1_Attack_MidiController:
                return pack_AR_RS_Value(OP_CONTROL(0, attack_rate) = INVERT(value) >> 2,
                                        OP_CONTROL(0, rate_scaling));
            case Op1_Decay1_MidiController:
                return OP_CONTROL(0, decay1_rate) = INVERT(value) >> 2;
            case Op1_Sustain_MidiController:
                return pack_RR_SL_Value(OP_CONTROL(0, release_rate),
                                        OP_CONTROL(0, sustain_level) = INVERT(value) >> 3);
            case Op1_Decay2_MidiController:
                return OP_CONTROL(0, decay2_rate) = INVERT(value) >> 2;
            case Op1_Release_MidiController:
                return pack_RR_SL_Value(OP_CONTROL(0, release_rate) = INVERT(value) >> 3,
                                        OP_CONTROL(0, sustain_level));
            case Op1_EnvScale_MidiController:
                return pack_AR_RS_Value(OP_CONTROL(0, attack_rate),
                                        OP_CONTROL(0, rate_scaling) = value >> 5);
            case Op2_Attack_MidiController:
                return pack_AR_RS_Value(OP_CONTROL(1, attack_rate) = INVERT(value) >> 2,
                                        OP_CONTROL(1, rate_scaling));
            case Op2_Decay1_MidiController:
                return OP_CONTROL(1, decay1_rate) = INVERT(value) >> 2;
            case Op2_Sustain_MidiController:
                return pack_RR_SL_Value(OP_CONTROL(1, release_rate),
                                        OP_CONTROL(1, sustain_level) = INVERT(value) >> 3);
            case Op2_Decay2_MidiController:
                return OP_CONTROL(1, decay2_rate) = INVERT(value) >> 2;
            case Op2_Release_MidiController:
                return pack_RR_SL_Value(OP_CONTROL(1, release_rate) = INVERT(value) >> 3,
                                        OP_CONTROL(1, sustain_level));
            case Op2_EnvScale_MidiController:
                return pack_AR_RS_Value(OP_CONTROL(1, attack_rate),
                                        OP_CONTROL(1, rate_scaling) = value >> 5);
            case Op3_Attack_MidiController:
                return pack_AR_RS_Value(OP_CONTROL(2, attack_rate) = INVERT(value) >> 2,
                                        OP_CONTROL(2, rate_scaling));
            case Op3_Decay1_MidiController:
                return OP_CONTROL(2, decay1_rate) = INVERT(value) >> 2;
            case Op3_Sustain_MidiController:
                return pack_RR_SL_Value(OP_CONTROL(2, release_rate),
                                        OP_CONTROL(2, sustain_level) = INVERT(value) >> 3);
            case Op3_Decay2_MidiController:
                return OP_CONTROL(2, decay2_rate) = INVERT(value) >> 2;
            case Op3_Release_MidiController:
                return pack_RR_SL_Value(OP_CONTROL(2, release_rate) = INVERT(value) >> 3,
                                        OP_CONTROL(2, sustain_level));
            case Op3_EnvScale_MidiController:
                return pack_AR_RS_Value(OP_CONTROL(2, attack_rate),
                                        OP_CONTROL(2, rate_scaling) = value >> 5);
            case Op4_Attack_MidiController:
                return pack_AR_RS_Value(OP_CONTROL(3, attack_rate) = INVERT(value) >> 2,
                                        OP_CONTROL(3, rate_scaling));
            case Op4_Decay1_MidiController:
                return OP_CONTROL(3, decay1_rate) = INVERT(value) >> 2;
            case Op4_Sustain_MidiController:
                return pack_RR_SL_Value(OP_CONTROL(3, release_rate),
                                        OP_CONTROL(3, sustain_level) = INVERT(value) >> 3);
            case Op4_Decay2_MidiController:
                return OP_CONTROL(3, decay2_rate) = INVERT(value) >> 2;
            case Op4_Release_MidiController:
                return pack_RR_SL_Value(OP_CONTROL(3, release_rate) = INVERT(value) >> 3,
                                        OP_CONTROL(3, sustain_level));
            case Op4_EnvScale_MidiController:
                return pack_AR_RS_Value(OP_CONTROL(3, attack_rate),
                                        OP_CONTROL(3, rate_scaling) = value >> 5);
        };
    }

    Voice::Voice(Controls &controls, int fm_channel)
    : m_controls(&controls), m_fm_channel(fm_channel), m_velocity(0)
    {
    }

    void Voice::setPitch(uint8_t note, int16_t bend_amount)
    {
        uint16_t freq;
        freq = noteToYM2612Frequency((float)note + ((float)bend_amount / 4096));
        setChannelRegister(0xa4, freq >> 8);
        setChannelRegister(0xa0, freq & 0xff);
    }

    void Voice::on(uint8_t velocity)
    {
        // Each operator may start at a different time so the note doesn't
        // really start here
        
        // TODO: SSG-EG

        m_velocity = velocity;

        setChannelRegister(ALGO_FEED_ChannelRegister, pack_ALGO_FEED_Value(
            m_controls->algorithm,
            m_controls->op1_feedback));

        // PM sensitivity will be enabled in Update() when PM start delay elapses
        m_pm_started = false;
        setChannelRegister(L_R_AMS_PMS_ChannelRegister, pack_L_R_AMS_PMS_Value(
            m_controls->left_output_enable,
            m_controls->right_output_enable,
            m_controls->am_sensitivity,
            0));

        for (int op = 0; op < 4; ++ op) {
            m_timed_controls[op].started = false;
            m_timed_controls[op].am_started = false;

            setOperatorRegister(op, MUL_DT_OperatorRegister, pack_MUL_DT_Value(
                m_controls->operators[op].frequency_multiplier,
                m_controls->operators[op].detune));

            uint8_t total_level = 127 - getScaledLevel(velocity, m_controls->operators[op].velocity_to_level,
                                                                 m_controls->operators[op].level);
            setOperatorRegister(op, TL_OperatorRegister, total_level);

            setOperatorRegister(op, AR_RS_OperatorRegister, pack_AR_RS_Value(
                m_controls->operators[op].attack_rate,
                m_controls->operators[op].rate_scaling));

            // AMON will be enabled in Update() when AM start delay elapses
            setOperatorRegister(op, DR_AMON_OperatorRegister, pack_DR_AMON_Value(
                m_controls->operators[op].decay1_rate,
                false));

            setOperatorRegister(op, SR_OperatorRegister,
                m_controls->operators[op].decay2_rate);

            setOperatorRegister(op, RR_SL_OperatorRegister, pack_RR_SL_Value(
                m_controls->operators[op].release_rate,
                m_controls->operators[op].sustain_level));

            // TODO: Maybe support this in future
            setOperatorRegister(op, SSGEG_OperatorRegister, 0x00);
        }
    }

    uint16_t Voice::off()
    {
        uint8_t ch = m_fm_channel;
        if (ch> 2) ++ ch;
        setGlobalRegister(Key_GlobalRegister, 0x00 | ch);

        // This prevents further key-ons during Update()
        for (int i = 0; i < 4; ++ i) {
            m_timed_controls[i].started = true;
        }

        return 2000;        // TODO: Base on the longest carrier release time
    }

    // Synth implementation will have packed a register value already so we
    // just need to figure out which register to update (an exception is total_level
    // which needs to take velocity and velocity sensitivity into consideration)
    void Voice::setControl(uint8_t control, int16_t value)
    {
        // Start times are set by Synth already and will be handled by Update()
        switch (control) {
            case Algorithm_MidiController:
            case Op1_Feedback_MidiController:
                setChannelRegister(ALGO_FEED_ChannelRegister, value);
                break;

            case AMDepth_MidiController:
                setChannelRegister(L_R_AMS_PMS_ChannelRegister, pack_L_R_AMS_PMS_Value(
                    m_controls->left_output_enable,
                    m_controls->right_output_enable,
                    m_controls->am_sensitivity,
                    m_pm_started ? m_controls->pm_sensitivity : 0
                ));
                break;

            case PMDepth_MidiController: {
                setChannelRegister(L_R_AMS_PMS_ChannelRegister, pack_L_R_AMS_PMS_Value(
                    m_controls->left_output_enable,
                    m_controls->right_output_enable,
                    m_controls->am_sensitivity,
                    m_pm_started ? m_controls->pm_sensitivity : 0
                ));
                break;
            }

            case Op1_Level_MidiController: {
            case Op1_Velocity_MidiController:
                uint8_t total_level = 127 - getScaledLevel(m_velocity, m_controls->operators[0].velocity_to_level,
                                                                       m_controls->operators[0].level);
                setOperatorRegister(0, TL_OperatorRegister, total_level);
                break;
            }

            case Op2_Level_MidiController: {
            case Op2_Velocity_MidiController:
                uint8_t total_level = 127 - getScaledLevel(m_velocity, m_controls->operators[1].velocity_to_level,
                                                                       m_controls->operators[1].level);
                setOperatorRegister(1, TL_OperatorRegister, total_level);
                break;
            }

            case Op3_Level_MidiController: {
            case Op3_Velocity_MidiController:
                uint8_t total_level = 127 - getScaledLevel(m_velocity, m_controls->operators[2].velocity_to_level,
                                                                       m_controls->operators[2].level);
                setOperatorRegister(2, TL_OperatorRegister, total_level);
                break;
            }

            case Op4_Level_MidiController: {
            case Op4_Velocity_MidiController:
                uint8_t total_level = 127 - getScaledLevel(m_velocity, m_controls->operators[3].velocity_to_level,
                                                                       m_controls->operators[3].level);
                setOperatorRegister(3, TL_OperatorRegister, total_level);
                break;
            }

            case Op1_FreqX_MidiController:
            case Op1_Detune_MidiController:
                setOperatorRegister(0, MUL_DT_OperatorRegister, value);
                break;

            case Op2_FreqX_MidiController:
            case Op2_Detune_MidiController:
                setOperatorRegister(1, MUL_DT_OperatorRegister, value);
                break;

            case Op3_FreqX_MidiController:
            case Op3_Detune_MidiController:
                setOperatorRegister(2, MUL_DT_OperatorRegister, value);
                break;

            case Op4_FreqX_MidiController:
            case Op4_Detune_MidiController:
                setOperatorRegister(3, MUL_DT_OperatorRegister, value);
                break;

            case Op1_Attack_MidiController:
            case Op1_EnvScale_MidiController:
                setOperatorRegister(0, AR_RS_OperatorRegister, value);
                break;

            case Op1_Decay1_MidiController:
                setOperatorRegister(0, DR_AMON_OperatorRegister, pack_DR_AMON_Value(
                    value,
                    m_timed_controls[0].am_started
                ));
                break;

            case Op1_Sustain_MidiController:
            case Op1_Release_MidiController:
                setOperatorRegister(0, RR_SL_OperatorRegister, value);
                break;

            case Op1_Decay2_MidiController:
                setOperatorRegister(0, SR_OperatorRegister, value);
                break;

            case Op2_Attack_MidiController:
            case Op2_EnvScale_MidiController:
                setOperatorRegister(1, AR_RS_OperatorRegister, value);
                break;

            case Op2_Decay1_MidiController:
                setOperatorRegister(1, DR_AMON_OperatorRegister, pack_DR_AMON_Value(
                    value,
                    m_timed_controls[1].am_started
                ));
                break;

            case Op2_Sustain_MidiController:
            case Op2_Release_MidiController:
                setOperatorRegister(1, RR_SL_OperatorRegister, value);
                break;

            case Op2_Decay2_MidiController:
                setOperatorRegister(1, SR_OperatorRegister, value);
                break;

            case Op3_Attack_MidiController:
            case Op3_EnvScale_MidiController:
                setOperatorRegister(2, AR_RS_OperatorRegister, value);
                break;

            case Op3_Decay1_MidiController:
                setOperatorRegister(2, DR_AMON_OperatorRegister, pack_DR_AMON_Value(
                    value,
                    m_timed_controls[2].am_started
                ));
                break;

            case Op3_Sustain_MidiController:
            case Op3_Release_MidiController:
                setOperatorRegister(2, RR_SL_OperatorRegister, value);
                break;

            case Op3_Decay2_MidiController:
                setOperatorRegister(2, SR_OperatorRegister, value);
                break;

            case Op4_Attack_MidiController:
            case Op4_EnvScale_MidiController:
                setOperatorRegister(3, AR_RS_OperatorRegister, value);
                break;

            case Op4_Decay1_MidiController:
                setOperatorRegister(3, DR_AMON_OperatorRegister, pack_DR_AMON_Value(
                    value,
                    m_timed_controls[3].am_started
                ));
                break;

            case Op4_Sustain_MidiController:
            case Op4_Release_MidiController:
                setOperatorRegister(3, RR_SL_OperatorRegister, value);
                break;

            case Op4_Decay2_MidiController:
                setOperatorRegister(3, SR_OperatorRegister, value);
                break;
        }
    }

    void Voice::update(uint32_t elapsed)
    {
        // Operator key and AM starts
        uint8_t key_reg_value = 0x00;
        bool update_key_reg = false;

        for (int op = 0; op < 4; ++ op) {
            // Diode clamp limits range, preventing a zero start time being possible with pot only
            // so need to ignore the lower-end
            uint16_t start_time = m_controls->operators[op].start;
            if (start_time < 4) {
                start_time = 0;
            } else {
                start_time -= 4;
            }

            // AM start doesn't have a CV jack so don't need to compensate for diode clamp
            uint16_t am_start_time = m_controls->operators[op].am_start;

            // Range: 0=0ms, 32=128ms, 64=512ms, 96=1152ms, 127=2016ms
            start_time *= start_time;
            start_time /= 8;

            // Same range as above, but offset by start time
            am_start_time *= am_start_time;
            am_start_time /= 8;
            am_start_time += start_time;

            if (elapsed >= start_time) {
                key_reg_value |= 0x10 << op;
                if (!m_timed_controls[op].started) {
                    m_timed_controls[op].started = true;
                    update_key_reg = true;
                }
            }

            if (elapsed >= am_start_time) {
                if (!m_timed_controls[op].am_started) {
                    m_timed_controls[op].am_started = true;
                    setOperatorRegister(op, DR_AMON_OperatorRegister, pack_DR_AMON_Value(
                        m_controls->operators[op].decay1_rate,
                        m_timed_controls[op].am_started));
                }
            }
        }

        uint16_t pm_start_time = m_controls->pm_start;
        pm_start_time *= pm_start_time;
        pm_start_time /= 8;

        if (elapsed > pm_start_time) {
            if (!m_pm_started) {
                m_pm_started = true;
                setChannelRegister(L_R_AMS_PMS_ChannelRegister, pack_L_R_AMS_PMS_Value(
                    m_controls->left_output_enable,
                    m_controls->right_output_enable,
                    m_controls->am_sensitivity,
                    m_controls->pm_sensitivity
                ));
            }
        }

        if (update_key_reg) {
            uint8_t ch = m_fm_channel;
            if (ch > 2) ++ ch;
            setGlobalRegister(Key_GlobalRegister, key_reg_value | ch);
        }
    }
};

MIDI_CREATE_DEFAULT_INSTANCE()

YM2612::Synth g_synth;

void setup()
{
    YM2612::init();

    MIDI.begin(MIDI_CHANNEL_OMNI);
    MIDI.turnThruOff();
}

int8_t test_note = 40;

void loop()
{
    if (MIDI.read()) {
        switch(MIDI.getType()) {
            case midi::NoteOn:
                g_synth.noteOn(0, MIDI.getData1(), MIDI.getData2());
                break;

            case midi::NoteOff:
                g_synth.noteOff(0, MIDI.getData1());
                break;
                
            case midi::ControlChange:
                g_synth.controlChange(0, MIDI.getData1(), MIDI.getData2());
                break;

            case midi::PitchBend:
                g_synth.pitchBend(0, (MIDI.getData2() << 7) | MIDI.getData1());
                break;
        }
    }

    g_synth.process();
}
