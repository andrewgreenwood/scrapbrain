/*
  PORTB = D8-D13
  PORTC = A0-A5
  PORTD = D0-D7
*/

#include <SoftwareSerial.h>
#include <MIDI.h>
#include <SPI.h>
#include "synth.h"

#define YM2612_CLOCK_PIN  9

#define YM2612_CS_595_PIN   0x02
#define YM2612_WR_595_PIN   0x04
#define YM2612_RD_595_PIN   0x08
#define YM2612_A0_595_PIN   0x10
#define YM2612_A1_595_PIN   0x20
#define YM2612_IC_595_PIN   0x80

void TestShift(uint8_t a)
{
    SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
    SPI.transfer(a);
    SPI.transfer(0x00);
    SPI.transfer(0x00);
    SPI.endTransaction();
    digitalWrite(10, HIGH);
    digitalWrite(10, LOW);
}

uint8_t g_shift_val_a = 0x00;

void shiftWrite(uint8_t p, uint8_t v)
{
    if (v == HIGH) {
        g_shift_val_a |= p;
    } else {
        g_shift_val_a &= ~p;
    }

    TestShift(g_shift_val_a);
}

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
          am_sensitivity(0), pm_sensitivity(0)
        {
            for (int op = 0; op < 4; ++ op) {
                operators[op].start = 0;
                operators[op].frequency_multiplier = 1;
                operators[op].detune = 0;
                operators[op].total_level = 0;
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
            }
        };

        uint8_t algorithm               : 3;
        uint8_t op1_feedback            : 3;
        uint8_t left_output_enable      : 1;
        uint8_t right_output_enable     : 1;
        uint8_t am_sensitivity          : 2;
        uint8_t pm_sensitivity          : 3;

        struct {
            uint8_t start                   : 7;
            uint8_t frequency_multiplier    : 4;
            uint8_t detune                  : 3;
            uint8_t total_level             : 7;
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

    static uint8_t Pack_ALGO_FEED_Value(uint8_t algo, uint8_t feed)
    { return (algo & 0x7) | ((feed & 0x7) << 3); }

    static uint8_t Pack_L_R_AMS_PMS_Value(uint8_t l, uint8_t r, uint8_t ams, uint8_t pms)
    { return (((l & 0x1) << 7) | ((r & 0x1) << 6) | ((ams & 0x3) << 4) | (pms & 0x7)); }

    enum OperatorRegister {
        MUL_DT_OperatorRegister = 0x30,
        TL_OperatorRegister = 0x40,
        AR_RS_OperatorRegister = 0x50,
        DR_AMON_OperatorRegister = 0x60,
        SR_OperatorRegister = 0x70,
        RR_SL_OperatorRegister = 0x80,
        SSGEG_OperatorRegister = 0x90,
    };

    static uint8_t Pack_MUL_DT_Value(uint8_t mul, uint8_t dt)
    { return (mul & 0x0f) | ((dt & 0x7) << 4); }

    static uint8_t Pack_AR_RS_Value(uint8_t ar, uint8_t rs)
    { return (ar & 0x1f) | ((rs & 0x3) << 6); }

    static uint8_t Pack_DR_AMON_Value(uint8_t dr, bool amon)
    { return (dr & 0x1f) | (amon << 7); }

    static uint8_t Pack_RR_SL_Value(uint8_t rr, uint8_t sl)
    { return (rr & 0xf) | ((sl & 0xf) << 4); }

    uint8_t Read()
    {
        shiftWrite(YM2612_CS_595_PIN, LOW);
        //digitalWrite(YM2612_CS_PIN, LOW);
        delayMicroseconds(1);

        // D0-D7 as inputs
        DDRD = 0x00;

        shiftWrite(YM2612_A0_595_PIN, LOW);
        shiftWrite(YM2612_A1_595_PIN, LOW);
//        digitalWrite(YM2612_A0_PIN, LOW);
        //digitalWrite(YM2612_A1_PIN, LOW);

        delayMicroseconds(1);
        shiftWrite(YM2612_RD_595_PIN, LOW);
        //digitalWrite(YM2612_RD_PIN, LOW);
        delayMicroseconds(5);

        uint8_t data = PORTD;

        shiftWrite(YM2612_RD_595_PIN, HIGH);
//        digitalWrite(YM2612_RD_PIN, HIGH);
        delayMicroseconds(5);

        shiftWrite(YM2612_CS_595_PIN, HIGH);
        //digitalWrite(YM2612_CS_PIN, HIGH);

        return data;
    }

    // Wait for YM2612 busy bit to be clear
    void Wait()
    {
        //while (Read() & 0x80) { }
    }

    void Write(uint8_t address, uint8_t data)
    {
        Wait();

        shiftWrite(YM2612_CS_595_PIN, LOW);
//        digitalWrite(YM2612_CS_PIN, LOW);
        delayMicroseconds(1);

        shiftWrite(YM2612_A0_595_PIN, address & 2 ? HIGH : LOW);
        shiftWrite(YM2612_A1_595_PIN, address & 1 ? HIGH : LOW);
//        digitalWrite(YM2612_A0_PIN, address & 2 ? HIGH : LOW);
//        digitalWrite(YM2612_A1_PIN, address & 1 ? HIGH : LOW);

        // D0-D7 as outputs
        DDRD = 0xff;

        PORTD = data;

        delayMicroseconds(1);
        shiftWrite(YM2612_WR_595_PIN, LOW);
//        digitalWrite(YM2612_WR_PIN, LOW);
        delayMicroseconds(5);

        shiftWrite(YM2612_WR_595_PIN, HIGH);
//        digitalWrite(YM2612_WR_PIN, HIGH);
        delayMicroseconds(5);

        shiftWrite(YM2612_CS_595_PIN, HIGH);
//        digitalWrite(YM2612_CS_PIN, HIGH);
    }

    void SetGlobalRegister(GlobalRegister reg, uint8_t value)
    {
        Write(0, reg);
        Write(2, value);
    }

    void Init()
    {
        //pinMode(YM2612_CS_PIN, OUTPUT);
        //pinMode(YM2612_RD_PIN, OUTPUT);
        //pinMode(YM2612_WR_PIN, OUTPUT);
        //pinMode(YM2612_A0_PIN, OUTPUT);
        //pinMode(YM2612_A1_PIN, OUTPUT);

        // D0-D7 as outputs
        DDRD |= 0xff;

        shiftWrite(YM2612_IC_595_PIN, HIGH);
        shiftWrite(YM2612_CS_595_PIN, HIGH);
        shiftWrite(YM2612_RD_595_PIN, HIGH);
        shiftWrite(YM2612_WR_595_PIN, HIGH);
        shiftWrite(YM2612_A0_595_PIN, LOW);
        shiftWrite(YM2612_A1_595_PIN, LOW);
        //digitalWrite(YM2612_CS_PIN, HIGH);
        //digitalWrite(YM2612_RD_PIN, HIGH);
        //digitalWrite(YM2612_WR_PIN, HIGH);
        //digitalWrite(YM2612_A0_PIN, LOW);
        //digitalWrite(YM2612_A1_PIN, LOW);

        // Output 8MHz PWM for clock
        pinMode(YM2612_CLOCK_PIN, OUTPUT);
        // Timer compare mode for channel A
        TCCR1A = _BV(COM1A0);
        // Timer control register B (waveform generation mode - CTC?, no prescaling)
        TCCR1B = _BV(WGM12) | _BV(CS10);
        // Timer control register C
        TCCR1C = 0;
        TCNT1 = 0;
        OCR1A = 0;

        // Initialise (NOTE: hardcoded pin!)    - DAC CS ?
        //pinMode(A0, OUTPUT);
        //digitalWrite(A0, LOW);
        //delay(10);
        //digitalWrite(A0, HIGH);
        //delay(10);

        // Reset and stop timers, set channel 3 to normal mode
        SetGlobalRegister(Channel3ModeAndTimer_GlobalRegister, 0x00);
        SetGlobalRegister(TimerAFrequencyHigh_GlobalRegister, 0x00);
        SetGlobalRegister(TimerAFrequencyLow_GlobalRegister, 0x00);
        SetGlobalRegister(TimerBFrequency_GlobalRegister, 0x00);

        // LFO off
        //SetGlobalRegister(LFO_GlobalRegister, 0x00);
        SetGlobalRegister(LFO_GlobalRegister, 0x08);        // TESTING ONLY

        // Note off (all channels)
        SetGlobalRegister(Key_GlobalRegister, 0x00);
        SetGlobalRegister(Key_GlobalRegister, 0x01);
        SetGlobalRegister(Key_GlobalRegister, 0x02);
        SetGlobalRegister(Key_GlobalRegister, 0x04);
        SetGlobalRegister(Key_GlobalRegister, 0x05);
        SetGlobalRegister(Key_GlobalRegister, 0x06);

        // Turn off DAC
        SetGlobalRegister(Key_GlobalRegister, 0x00);
        SetGlobalRegister(DACEnable_GlobalRegister, 0x00);
    }

    class Voice: public SynthNote {
        public:
            Voice()
            : m_controls(NULL), m_fm_channel(-1)
            {
            }

            Voice(Controls &controls, int fm_channel);

            virtual ~Voice()
            { }

            virtual void SetPitch(uint8_t note, int16_t bend_amount);

            virtual void On(uint8_t velocity);

            virtual uint16_t Off();

            virtual void End()
            {
            }

            virtual void SetControl(uint8_t control, int16_t value);

            virtual void Update(uint32_t elapsed);

        private:
            void UpdateChannelRegister(uint8_t reg);

            void UpdateOperatorRegister(uint8_t op, uint8_t reg);

            void SetChannelRegister(ChannelRegister reg, uint8_t value)
            {
                int base_address = m_fm_channel < 3 ? 0 : 1;
                Write(base_address, reg + (m_fm_channel % 3));
                Write(base_address + 2, value);
            }

            void SetOperatorRegister(uint8_t op, OperatorRegister reg, uint8_t value)
            {
                static uint8_t reg_offset[4] = {0x0, 0x8, 0x4, 0xc};
                int base_address = m_fm_channel < 3 ? 0 : 1;
                Write(base_address, reg + (m_fm_channel % 3) + reg_offset[op]);
                Write(base_address + 2, value);
            }

            Controls *m_controls;
            int m_fm_channel;
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
                        //m_controls[ch].operators[op].am_start = 10 * (4 - op);      // test
                    }
                }
            }

            virtual ~Synth()
            { }

            static void Init();

            virtual SynthNote* AllocateNote(uint8_t channel);

            virtual void FreeNote(SynthNote *voice);

            virtual int16_t SetControl(uint8_t channel, uint8_t control, uint8_t value);

        private:
            Voice m_voices[6];
            uint8_t m_allocated_voice_bitmap;
            Controls m_controls[16];
    };

    SynthNote* Synth::AllocateNote(uint8_t channel)
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

    void Synth::FreeNote(SynthNote *voice)
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
    int16_t Synth::SetControl(uint8_t channel, uint8_t control, uint8_t value)
    {
        #define INVERT(v) \
            (127-v)

        #define CHANNEL_CONTROL(prop) \
            m_controls[channel].prop

        #define OP_CONTROL(op, prop) \ 
            m_controls[channel].operators[op].prop

        switch (control) {
            case 3:
                SetGlobalRegister(LFO_GlobalRegister, ScaleControlValue(value, 7, 15));
                break;

            case 9:
                return Pack_ALGO_FEED_Value(CHANNEL_CONTROL(algorithm) = value >> 4,
                                            CHANNEL_CONTROL(op1_feedback));
            case 10:
                return Pack_L_R_AMS_PMS_Value(CHANNEL_CONTROL(left_output_enable) = value <= 84,
                                              CHANNEL_CONTROL(right_output_enable) = value >= 42,
                                              CHANNEL_CONTROL(am_sensitivity),
                                              CHANNEL_CONTROL(pm_sensitivity));
            case 14:
                return Pack_L_R_AMS_PMS_Value(CHANNEL_CONTROL(left_output_enable),
                                              CHANNEL_CONTROL(right_output_enable),
                                              CHANNEL_CONTROL(am_sensitivity),
                                              CHANNEL_CONTROL(pm_sensitivity) >> 4);
            case 15:
                return Pack_L_R_AMS_PMS_Value(CHANNEL_CONTROL(left_output_enable),
                                              CHANNEL_CONTROL(right_output_enable),
                                              CHANNEL_CONTROL(am_sensitivity) >> 5,
                                              CHANNEL_CONTROL(pm_sensitivity));
            case 16:
                return OP_CONTROL(0, am_start) = value;
            case 17:
                return OP_CONTROL(1, am_start) = value;
            case 18:
                return OP_CONTROL(2, am_start) = value;
            case 19:
                return OP_CONTROL(3, am_start) = value;
            case 20:
                return OP_CONTROL(0, total_level) = value;
            case 21:
                return OP_CONTROL(1, total_level) = value;
            case 22:
                return OP_CONTROL(2, total_level) = value;
            case 23:
                return OP_CONTROL(3, total_level) = value;
            case 24:
                return OP_CONTROL(0, velocity_to_level) = value;
            case 25:
                return OP_CONTROL(1, velocity_to_level) = value;
            case 26:
                return OP_CONTROL(2, velocity_to_level) = value;
            case 27:
                return OP_CONTROL(3, velocity_to_level) = value;
            case 28:
                return OP_CONTROL(0, start) = value;
            case 29:
                return OP_CONTROL(1, start) = value;
            case 30:
                return OP_CONTROL(2, start) = value;
            case 31:
                return OP_CONTROL(3, start) = value;
            case 70:
                return Pack_MUL_DT_Value(OP_CONTROL(0, frequency_multiplier) = value >> 3,
                                         OP_CONTROL(0, detune));
            case 71:
                return Pack_MUL_DT_Value(OP_CONTROL(1, frequency_multiplier) = value >> 3,
                                         OP_CONTROL(1, detune));
            case 72:
                return Pack_MUL_DT_Value(OP_CONTROL(2, frequency_multiplier) = value >> 3,
                                         OP_CONTROL(2, detune));
            case 73:
                return Pack_MUL_DT_Value(OP_CONTROL(3, frequency_multiplier) = value >> 3,
                                         OP_CONTROL(3, detune));
            case 74:
                return Pack_MUL_DT_Value(OP_CONTROL(0, frequency_multiplier),
                                         OP_CONTROL(0, detune) = (7 - ScaleControlValue(value, 1, 7)));
            case 75:
                return Pack_MUL_DT_Value(OP_CONTROL(1, frequency_multiplier),
                                         OP_CONTROL(1, detune) = (7 - ScaleControlValue(value, 1, 7)));
            case 76:
                return Pack_MUL_DT_Value(OP_CONTROL(2, frequency_multiplier),
                                         OP_CONTROL(2, detune) = (7 - ScaleControlValue(value, 1, 7)));
            case 77:
                return Pack_MUL_DT_Value(OP_CONTROL(3, frequency_multiplier),
                                         OP_CONTROL(3, detune) = (7 - ScaleControlValue(value, 1, 7)));
            case 78:
                return Pack_ALGO_FEED_Value(CHANNEL_CONTROL(algorithm),
                                            CHANNEL_CONTROL(op1_feedback) = value >> 4);
            case 85:
                return Pack_AR_RS_Value(OP_CONTROL(0, attack_rate) = INVERT(value) >> 2,
                                        OP_CONTROL(0, rate_scaling));
            case 86:
                return OP_CONTROL(0, decay1_rate) = INVERT(value) >> 2;
            case 87:
                return Pack_RR_SL_Value(OP_CONTROL(0, release_rate),
                                        OP_CONTROL(0, sustain_level) = INVERT(value) >> 3);
            case 88:
                return OP_CONTROL(0, decay2_rate) = INVERT(value) >> 2;
            case 89:
                return Pack_RR_SL_Value(OP_CONTROL(0, release_rate) = INVERT(value) >> 3,
                                        OP_CONTROL(0, sustain_level));
            case 90:
                return Pack_AR_RS_Value(OP_CONTROL(0, attack_rate),
                                        OP_CONTROL(0, rate_scaling) = value >> 5);
            case 102:
                return Pack_AR_RS_Value(OP_CONTROL(1, attack_rate) = INVERT(value) >> 2,
                                        OP_CONTROL(1, rate_scaling));
            case 103:
                return OP_CONTROL(1, decay1_rate) = INVERT(value) >> 2;
            case 104:
                return Pack_RR_SL_Value(OP_CONTROL(1, release_rate),
                                        OP_CONTROL(1, sustain_level) = INVERT(value) >> 3);
            case 105:
                return OP_CONTROL(1, decay2_rate) = INVERT(value) >> 2;
            case 106:
                return Pack_RR_SL_Value(OP_CONTROL(1, release_rate) = INVERT(value) >> 3,
                                        OP_CONTROL(1, sustain_level));
            case 107:
                return Pack_AR_RS_Value(OP_CONTROL(1, attack_rate),
                                        OP_CONTROL(1, rate_scaling) = value >> 5);
            case 108:
                return Pack_AR_RS_Value(OP_CONTROL(2, attack_rate) = INVERT(value) >> 2,
                                        OP_CONTROL(2, rate_scaling));
            case 109:
                return OP_CONTROL(2, decay1_rate) = INVERT(value) >> 2;
            case 110:
                return Pack_RR_SL_Value(OP_CONTROL(2, release_rate),
                                        OP_CONTROL(2, sustain_level) = INVERT(value) >> 3);
            case 111:
                return OP_CONTROL(2, decay2_rate) = INVERT(value) >> 2;
            case 112:
                return Pack_RR_SL_Value(OP_CONTROL(2, release_rate) = INVERT(value) >> 3,
                                        OP_CONTROL(2, sustain_level));
            case 113:
                return Pack_AR_RS_Value(OP_CONTROL(2, attack_rate),
                                        OP_CONTROL(2, rate_scaling) = value >> 5);
            case 114:
                return Pack_AR_RS_Value(OP_CONTROL(3, attack_rate) = INVERT(value) >> 2,
                                        OP_CONTROL(3, rate_scaling));
            case 115:
                return OP_CONTROL(3, decay1_rate) = INVERT(value) >> 2;
            case 116:
                return Pack_RR_SL_Value(OP_CONTROL(3, release_rate),
                                        OP_CONTROL(3, sustain_level) = INVERT(value) >> 3);
            case 117:
                return OP_CONTROL(3, decay2_rate) = INVERT(value) >> 2;
            case 118:
                return Pack_RR_SL_Value(OP_CONTROL(3, release_rate) = INVERT(value) >> 3,
                                        OP_CONTROL(3, sustain_level));
            case 119:
                return Pack_AR_RS_Value(OP_CONTROL(3, attack_rate),
                                        OP_CONTROL(3, rate_scaling) = value >> 5);
        };
    }

    Voice::Voice(Controls &controls, int fm_channel)
    : m_controls(&controls), m_fm_channel(fm_channel)
    {
        // TODO: initialise controls based on the arg given

        //for (int i = 0; i < 4; ++ i) {
            //m_timed_controls[i].started = false;
            //m_timed_controls[i].am_started = false;
        //}

        // TODO: Move channel/operator init into Voice::On

        //SetChannelRegister(0xb0, 0x03);     // ALGO/FEEDBACK
        SetChannelRegister(0xb0, 0x07);     // ALGO/FEEDBACK  - useful for testing as all ops are carriers

        for (int op = 0; op < 4; ++ op) {
            //SetOperatorRegister(op, 0x30, 0x01);    // MUL/DT
            SetOperatorRegister(op, 0x40, 0x20);    // TL
            //SetOperatorRegister(op, 0x50, 0x0b);    // AR/RS
            //SetOperatorRegister(op, 0x60, 0x00);    // DR/AMON
            //SetOperatorRegister(op, 0x70, 0x00);    // SR
            //SetOperatorRegister(op, 0x80, 0x05);    // RR/SL
            SetOperatorRegister(op, 0x90, 0x00);    // SSG-EG
        }

        // TODO: Set output/ams/pms properly
        //SetChannelRegister(0xb4, 0xf0);
    }

    void Voice::SetPitch(uint8_t note, int16_t bend_amount)
    {
        uint16_t freq;
        freq = noteToYM2612Frequency((float)note + ((float)bend_amount / 4096));
        SetChannelRegister(0xa4, freq >> 8);
        SetChannelRegister(0xa0, freq & 0xff);
    }

    void Voice::On(uint8_t velocity)
    {
        // Each operator may start at a different time so the note doesn't
        // really start here
        
        // TODO: store the velocity and use this to calculate the level for
        // each operator
        // TODO: SSG-EG

        SetChannelRegister(ALGO_FEED_ChannelRegister, Pack_ALGO_FEED_Value(
            m_controls->algorithm,
            m_controls->op1_feedback));

        SetChannelRegister(L_R_AMS_PMS_ChannelRegister, Pack_L_R_AMS_PMS_Value(
            m_controls->left_output_enable,
            m_controls->right_output_enable,
            m_controls->am_sensitivity,
            m_controls->pm_sensitivity));

        for (int op = 0; op < 4; ++ op) {
            m_timed_controls[op].started = false;
            m_timed_controls[op].am_started = false;

            SetOperatorRegister(op, MUL_DT_OperatorRegister, Pack_MUL_DT_Value(
                m_controls->operators[op].frequency_multiplier,
                m_controls->operators[op].detune));

        #if 0
            // TODO: Total level needs to consider velo-to-level, velocity
            SetOperatorRegister(op, TL_OperatorRegister, 0);
        #endif

            SetOperatorRegister(op, AR_RS_OperatorRegister, Pack_AR_RS_Value(
                m_controls->operators[op].attack_rate,
                m_controls->operators[op].rate_scaling));

            // AMON will be enabled in Update() when AM start delay elapses
            SetOperatorRegister(op, DR_AMON_OperatorRegister, Pack_DR_AMON_Value(
                m_controls->operators[op].decay1_rate,
                false));

            SetOperatorRegister(op, SR_OperatorRegister,
                m_controls->operators[op].decay2_rate);

            SetOperatorRegister(op, RR_SL_OperatorRegister, Pack_RR_SL_Value(
                m_controls->operators[op].release_rate,
                m_controls->operators[op].sustain_level));
        }
    }

    uint16_t Voice::Off()
    {
        uint8_t ch = m_fm_channel;
        if (ch> 2) ++ ch;
        SetGlobalRegister(Key_GlobalRegister, 0x00 | ch);

        // This prevents further key-ons during Update()
        for (int i = 0; i < 4; ++ i) {
            m_timed_controls[i].started = true;
        }

        return 2000;        // TODO
    }

    // Synth implementation will have packed a register value already so we
    // just need to figure out which register to update
    void Voice::SetControl(uint8_t control, int16_t value)
    {
        // Start times are set by Synth already and will be handled by Update()
        switch (control) {
            case 9:
            case 78:
                SetChannelRegister(ALGO_FEED_ChannelRegister, value);
                break;

            case 10:
            case 14:
            case 15:
                SetChannelRegister(L_R_AMS_PMS_ChannelRegister, value);
                break;

            // TODO: Level, velocity to level

            case 70:
            case 74:
                SetOperatorRegister(0, MUL_DT_OperatorRegister, value);
                break;

            case 71:
            case 75:
                SetOperatorRegister(1, MUL_DT_OperatorRegister, value);
                break;

            case 72:
            case 76:
                SetOperatorRegister(2, MUL_DT_OperatorRegister, value);
                break;

            case 73:
            case 77:
                SetOperatorRegister(3, MUL_DT_OperatorRegister, value);
                break;

            case 85:
            case 90:
                SetOperatorRegister(0, AR_RS_OperatorRegister, value);
                break;

            case 86:
                SetOperatorRegister(0, DR_AMON_OperatorRegister, Pack_DR_AMON_Value(
                    value,
                    m_timed_controls[0].am_started
                ));
                break;

            case 87:
            case 89:
                SetOperatorRegister(0, RR_SL_OperatorRegister, value);
                break;

            case 88:
                SetOperatorRegister(0, SR_OperatorRegister, value);
                break;

            case 102:
            case 107:
                SetOperatorRegister(1, AR_RS_OperatorRegister, value);
                break;

            case 103:
                SetOperatorRegister(1, DR_AMON_OperatorRegister, Pack_DR_AMON_Value(
                    value,
                    m_timed_controls[1].am_started
                ));
                break;

            case 104:
            case 106:
                SetOperatorRegister(1, RR_SL_OperatorRegister, value);
                break;

            case 105:
                SetOperatorRegister(1, SR_OperatorRegister, value);
                break;

            case 108:
            case 113:
                SetOperatorRegister(2, AR_RS_OperatorRegister, value);
                break;

            case 109:
                SetOperatorRegister(2, DR_AMON_OperatorRegister, Pack_DR_AMON_Value(
                    value,
                    m_timed_controls[2].am_started
                ));
                break;

            case 110:
            case 112:
                SetOperatorRegister(2, RR_SL_OperatorRegister, value);
                break;

            case 111:
                SetOperatorRegister(2, SR_OperatorRegister, value);
                break;

            case 114:
            case 119:
                SetOperatorRegister(3, AR_RS_OperatorRegister, value);
                break;

            case 115:
                SetOperatorRegister(3, DR_AMON_OperatorRegister, Pack_DR_AMON_Value(
                    value,
                    m_timed_controls[3].am_started
                ));
                break;

            case 116:
            case 118:
                SetOperatorRegister(3, RR_SL_OperatorRegister, value);
                break;

            case 117:
                SetOperatorRegister(3, SR_OperatorRegister, value);
                break;
        }
    }

    void Voice::Update(uint32_t elapsed)
    {
        // Operator key and AM starts
        uint8_t key_reg_value = 0x00;
        bool update_key_reg = false;

        for (int op = 0; op < 4; ++ op) {
            if (elapsed >= (uint16_t)m_controls->operators[op].start * 100) {
                key_reg_value |= 0x10 << op;
                if (!m_timed_controls[op].started) {
                    m_timed_controls[op].started = true;
                    update_key_reg = true;
                }
            }

            if (elapsed >= (uint16_t)m_controls->operators[op].am_start * 100) {
                if (!m_timed_controls[op].am_started) {
                    m_timed_controls[op].am_started = true;
                    SetOperatorRegister(op, DR_AMON_OperatorRegister, Pack_DR_AMON_Value(
                        m_controls->operators[op].decay1_rate,
                        m_timed_controls[op].am_started));
                }
            }
        }

        if (update_key_reg) {
            uint8_t ch = m_fm_channel;
            if (ch > 2) ++ ch;
            SetGlobalRegister(Key_GlobalRegister, key_reg_value | ch);
        }
    }
};



SoftwareSerial midiInSerialPort(A1, 255);
MIDI_CREATE_INSTANCE(SoftwareSerial, midiInSerialPort, MIDI);

YM2612::Synth g_synth;


void setup() {
    // D0-D7 as inputs initially
    DDRD = 0;

    // Mux output inhibit
    pinMode(8, OUTPUT);

    // Pin 9 is used for YM2612 clock
    // (configured in YM2612::Init)

    // Shared 595 RCLK
    pinMode(10, OUTPUT);

    // SPI MOSI: DAC SDI and first 595 SER
    pinMode(11, OUTPUT);

    // SPI MISO: unused
    pinMode(12, INPUT);

    // SPI CLK
    pinMode(13, OUTPUT);

    // Mux inputs
    pinMode(A2, INPUT);
    pinMode(A3, INPUT);
    pinMode(A4, INPUT);
    pinMode(A5, INPUT);

    // Inhibit mux output (NOT)
    digitalWrite(8, LOW);

    YM2612::Init();

    MIDI.begin();
    MIDI.turnThruOff();
}



uint8_t CalcPotVal(uint16_t raw_value, uint8_t steps)
{
    uint16_t value = 0;

    // Some pots don't seem to go fully to zero
    if (raw_value >= 20) raw_value -= 20;

    if (steps <= 32) {
        // Make step changes line up with the tick markers
        if (raw_value < 130) {
            // Close to (or in) dead zone
            value = 0;
        } else if (raw_value < 210) {
            // Dead zone at low end means less range here
            value = (raw_value - 130) / 5;
        } else if (raw_value < 4900) {
            // Main zone
            value = 13 + ((raw_value - 210) / 47);
        } else if (raw_value < 4970) {
            // Dead zone at high end means less range here
            value = 112 + ((raw_value - 4900) / 8);
            //value = 0;
        } else {
            // Close to (or in) dead zone
            value = 127;
        }

        value /= (128 / steps);

        if (value >= steps) value = steps - 1;
    } else {
        value = raw_value / 39;

        if (value > 127) {
            value = 127;
        }
    }

    return value;
}

enum ControllerNumber {
    MIDI_CC_LFO_RATE = 3,
    MIDI_CC_ALGORITHM = 9,
    MIDI_CC_AMS = 14,
    MIDI_CC_PMS = 15,
    MIDI_CC_OP1_AM_START = 16,
    MIDI_CC_OP2_AM_START = 17,
    MIDI_CC_OP3_AM_START = 18,
    MIDI_CC_OP4_AM_START = 19,
    MIDI_CC_OP1_LEVEL = 20,
    MIDI_CC_OP2_LEVEL = 21,
    MIDI_CC_OP3_LEVEL = 22,
    MIDI_CC_OP4_LEVEL = 23,
    MIDI_CC_OP1_VELOCITY_TO_LEVEL = 24,
    MIDI_CC_OP2_VELOCITY_TO_LEVEL = 25,
    MIDI_CC_OP3_VELOCITY_TO_LEVEL = 26,
    MIDI_CC_OP4_VELOCITY_TO_LEVEL = 27,
    MIDI_CC_OP1_START = 28,
    MIDI_CC_OP2_START = 29,
    MIDI_CC_OP3_START = 30,
    MIDI_CC_OP4_START = 31,
    MIDI_CC_OP1_FREQUENCY_MULTIPLIER = 70,
    MIDI_CC_OP2_FREQUENCY_MULTIPLIER = 71,
    MIDI_CC_OP3_FREQUENCY_MULTIPLIER = 72,
    MIDI_CC_OP4_FREQUENCY_MULTIPLIER = 73,
    MIDI_CC_OP1_DETUNE = 74,
    MIDI_CC_OP2_DETUNE = 75,
    MIDI_CC_OP3_DETUNE = 76,
    MIDI_CC_OP4_DETUNE = 77,
    MIDI_CC_OP1_FEEDBACK = 78,
    MIDI_CC_OP1_ATTACK = 85,
    MIDI_CC_OP1_DECAY_1 = 86,
    MIDI_CC_OP1_SUSTAIN = 87,
    MIDI_CC_OP1_DECAY_2 = 88,
    MIDI_CC_OP1_RELEASE = 89,
    MIDI_CC_OP1_ENVELOPE_SCALING = 90,
    MIDI_CC_OP2_ATTACK = 102,
    MIDI_CC_OP2_DECAY_1 = 103,
    MIDI_CC_OP2_SUSTAIN = 104,
    MIDI_CC_OP2_DECAY_2 = 105,
    MIDI_CC_OP2_RELEASE = 106,
    MIDI_CC_OP2_ENVELOPE_SCALING = 107,
    MIDI_CC_OP3_ATTACK = 108,
    MIDI_CC_OP3_DECAY_1 = 109,
    MIDI_CC_OP3_SUSTAIN = 110,
    MIDI_CC_OP3_DECAY_2 = 111,
    MIDI_CC_OP3_RELEASE = 112,
    MIDI_CC_OP3_ENVELOPE_SCALING = 113,
    MIDI_CC_OP4_ATTACK = 114,
    MIDI_CC_OP4_DECAY_1 = 115,
    MIDI_CC_OP4_SUSTAIN = 116,
    MIDI_CC_OP4_DECAY_2 = 117,
    MIDI_CC_OP4_RELEASE = 118,
    MIDI_CC_OP4_ENVELOPE_SCALING = 119
};

#define PACK_CONTROLLER_MAPPING(pin, channel, controller, steps) \
    (((uint32_t)pin << 24) | ((uint32_t)channel << 16) | ((uint32_t)controller << 8) | steps)

#define UNPACK_CONTROLLER_MAPPING_MUX_PIN(mapping)      (mapping >> 24)
#define UNPACK_CONTROLLER_MAPPING_MUX_CHANNEL(mapping)  ((mapping >> 16) & 0x0f)
#define UNPACK_CONTROLLER_MAPPING_CONTROLLER(mapping)   ((mapping >> 8) & 0x7f)
#define UNPACK_CONTROLLER_MAPPING_STEPS(mapping)        (mapping & 0xff)


#define CONTROLLER_COUNT 48
const uint32_t g_controller_map[CONTROLLER_COUNT] PROGMEM = {
    PACK_CONTROLLER_MAPPING(A3, 0,  MIDI_CC_OP2_SUSTAIN, 16),
    PACK_CONTROLLER_MAPPING(A5, 0,  MIDI_CC_OP4_SUSTAIN, 16),

    PACK_CONTROLLER_MAPPING(A3, 1,  MIDI_CC_OP2_DECAY_2, 32),
    PACK_CONTROLLER_MAPPING(A5, 1,  MIDI_CC_OP4_DECAY_2, 32),

    PACK_CONTROLLER_MAPPING(A3, 2,  MIDI_CC_OP2_RELEASE, 16),
    PACK_CONTROLLER_MAPPING(A5, 2,  MIDI_CC_OP4_RELEASE, 16),

    PACK_CONTROLLER_MAPPING(A3, 3,  MIDI_CC_OP2_AM_START, 128),
    PACK_CONTROLLER_MAPPING(A5, 3,  MIDI_CC_OP4_AM_START, 128),

    PACK_CONTROLLER_MAPPING(A3, 4,  MIDI_CC_OP2_LEVEL, 128),
    PACK_CONTROLLER_MAPPING(A5, 4,  MIDI_CC_OP4_LEVEL, 128),

    PACK_CONTROLLER_MAPPING(A3, 5,  MIDI_CC_OP2_START, 128),
    PACK_CONTROLLER_MAPPING(A5, 5,  MIDI_CC_OP4_START, 128),

    PACK_CONTROLLER_MAPPING(A3, 6,  MIDI_CC_OP2_VELOCITY_TO_LEVEL, 128),
    PACK_CONTROLLER_MAPPING(A5, 6,  MIDI_CC_OP4_VELOCITY_TO_LEVEL, 128),

    PACK_CONTROLLER_MAPPING(A3, 7,  MIDI_CC_OP2_ENVELOPE_SCALING, 4),
    PACK_CONTROLLER_MAPPING(A5, 7,  MIDI_CC_OP4_ENVELOPE_SCALING, 4),

    PACK_CONTROLLER_MAPPING(A2, 8,  MIDI_CC_OP1_SUSTAIN, 16),
    PACK_CONTROLLER_MAPPING(A3, 8,  MIDI_CC_OP1_ATTACK, 32),
    PACK_CONTROLLER_MAPPING(A4, 8,  MIDI_CC_OP3_SUSTAIN, 16),
    PACK_CONTROLLER_MAPPING(A5, 8,  MIDI_CC_OP3_ATTACK, 32),

    PACK_CONTROLLER_MAPPING(A2, 9,  MIDI_CC_OP1_DECAY_2, 32),
    PACK_CONTROLLER_MAPPING(A3, 9,  MIDI_CC_OP1_DETUNE, 7),
    PACK_CONTROLLER_MAPPING(A4, 9,  MIDI_CC_OP3_DECAY_2, 32),
    PACK_CONTROLLER_MAPPING(A5, 9,  MIDI_CC_OP3_DETUNE, 7),

    PACK_CONTROLLER_MAPPING(A2, 10, MIDI_CC_OP1_RELEASE, 16),
    PACK_CONTROLLER_MAPPING(A3, 10, MIDI_CC_OP1_DECAY_1, 32),
    PACK_CONTROLLER_MAPPING(A4, 10, MIDI_CC_OP3_RELEASE, 16),
    PACK_CONTROLLER_MAPPING(A5, 10, MIDI_CC_OP3_DECAY_1, 32),

    PACK_CONTROLLER_MAPPING(A2, 11, MIDI_CC_OP1_AM_START, 128),
    PACK_CONTROLLER_MAPPING(A3, 11, MIDI_CC_OP1_FREQUENCY_MULTIPLIER, 16),
    PACK_CONTROLLER_MAPPING(A4, 11, MIDI_CC_OP3_AM_START, 128),
    PACK_CONTROLLER_MAPPING(A5, 11, MIDI_CC_OP3_FREQUENCY_MULTIPLIER, 16),

    PACK_CONTROLLER_MAPPING(A2, 12, MIDI_CC_OP1_LEVEL, 128),
    PACK_CONTROLLER_MAPPING(A3, 12, MIDI_CC_OP2_ATTACK, 32),
    PACK_CONTROLLER_MAPPING(A4, 12, MIDI_CC_OP3_LEVEL, 128),
    PACK_CONTROLLER_MAPPING(A5, 12, MIDI_CC_OP4_ATTACK, 32),

    PACK_CONTROLLER_MAPPING(A2, 13, MIDI_CC_OP1_START, 128),
    PACK_CONTROLLER_MAPPING(A3, 13, MIDI_CC_OP2_DETUNE, 7),
    PACK_CONTROLLER_MAPPING(A4, 13, MIDI_CC_OP3_START, 128),
    PACK_CONTROLLER_MAPPING(A5, 13, MIDI_CC_OP4_DETUNE, 7),

    PACK_CONTROLLER_MAPPING(A2, 14, MIDI_CC_OP1_VELOCITY_TO_LEVEL, 128),
    PACK_CONTROLLER_MAPPING(A3, 14, MIDI_CC_OP2_DECAY_1, 32),
    PACK_CONTROLLER_MAPPING(A4, 14, MIDI_CC_OP3_VELOCITY_TO_LEVEL, 128),
    PACK_CONTROLLER_MAPPING(A5, 14, MIDI_CC_OP4_DECAY_1, 32),

    PACK_CONTROLLER_MAPPING(A2, 15, MIDI_CC_OP1_ENVELOPE_SCALING, 4),
    PACK_CONTROLLER_MAPPING(A3, 15, MIDI_CC_OP2_FREQUENCY_MULTIPLIER, 16),
    PACK_CONTROLLER_MAPPING(A4, 15, MIDI_CC_OP3_ENVELOPE_SCALING, 4),
    PACK_CONTROLLER_MAPPING(A5, 15, MIDI_CC_OP4_FREQUENCY_MULTIPLIER, 16)
};


uint16_t g_control_readings[48][5];
int g_control_reading_index = 0;
uint16_t g_control_last_values[48];

void loop()
{
    for (int i = 0; i < CONTROLLER_COUNT; ++ i) {
        uint32_t mapping = pgm_read_dword(&(g_controller_map[i]));
        
        // Select multiplexer channel and take reading
        digitalWrite(8, LOW);
        DDRD = 0x0f;
        PORTD = UNPACK_CONTROLLER_MAPPING_MUX_CHANNEL(mapping);
        uint8_t pin = UNPACK_CONTROLLER_MAPPING_MUX_PIN(mapping);
        uint16_t reading = analogRead(pin);
        digitalWrite(8, HIGH);

        // Store the reading
        g_control_readings[i][g_control_reading_index] = reading;
        if (++ g_control_reading_index == 5) g_control_reading_index = 0;

        // Total up the readings (sort of works like an average but without doing the division)
        reading = 0;
        for (int j = 0; j < 5; ++ j) {
            reading += g_control_readings[i][j];
        }

        // Update controls that have changed position by a significant amount (avoiding noise/jitter)
        if ((reading < g_control_last_values[i] - 8) || (reading > g_control_last_values[i] + 8)) {
            uint8_t steps = UNPACK_CONTROLLER_MAPPING_STEPS(mapping);
            uint8_t value = CalcPotVal(reading, steps);

            if (value != CalcPotVal(g_control_last_values[i], steps)) {
                uint8_t controller = UNPACK_CONTROLLER_MAPPING_CONTROLLER(mapping);
                //if (controller != MIDI_CC_OP1_FREQUENCY_MULTIPLIER) continue;

                switch (controller) {
                    case MIDI_CC_OP1_START:
                    case MIDI_CC_OP2_START:
                    case MIDI_CC_OP3_START:
                    case MIDI_CC_OP4_START:
                    case MIDI_CC_OP1_LEVEL:
                    case MIDI_CC_OP2_LEVEL:
                    case MIDI_CC_OP3_LEVEL:
                    case MIDI_CC_OP4_LEVEL:
                        // Compensate for protection circuitry resistance
                        if (value < 3) value = 3;
                        value = (float)(value - 3) * 1.03;
                        if (value > 127) value = 127;
                    default:
                        break;
                }

                g_synth.ControlChange(0, UNPACK_CONTROLLER_MAPPING_CONTROLLER(mapping), value * (128/steps));
                g_control_last_values[i] = value;
            }
        }

        // Process any incoming MIDI data
        if (MIDI.read()) {
            switch(MIDI.getType()) {
                case midi::NoteOn:
                    g_synth.NoteOn(0, MIDI.getData1(), MIDI.getData2());
                    break;

                case midi::NoteOff:
                    g_synth.NoteOff(0, MIDI.getData1());
                    break;
                    
                case midi::ControlChange:
                    g_synth.ControlChange(0, MIDI.getData1(), MIDI.getData2());
                    break;

                case midi::PitchBend:
                    g_synth.PitchBend(0, (MIDI.getData2() << 7) | MIDI.getData1());
                    break;
            }
        }

        g_synth.Process();
    }
}
