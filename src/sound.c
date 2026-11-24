/*
 * Sound synthesis: 8-bit style effects and menu music.
 *
 * Every sound is built from "tones": a waveform at a frequency for a
 * duration, optionally sliding to another frequency. Effects are
 * short tables of tones played one after another; the music is a
 * table of notes on a fixed beat. Changing a sound means editing a
 * table, not the code.
 *
 * All audio here is synthesised by this program (no recorded files),
 * so it is original work under the project's MIT licence.
 */
#include "sound.h"

#include <stdlib.h>

typedef enum {
    WAVE_PULSE,      /* 25% pulse: bright, classic 8-bit lead */
    WAVE_SQUARE,     /* 50% square: buzzy                      */
    WAVE_TRIANGLE    /* soft, used for bass and gentle notes   */
} Wave;

typedef struct {
    double frequency;       /* Hz; 0 means silence (a rest)         */
    double end_frequency;   /* Hz to slide to; 0 means no slide     */
    int duration_ms;
    Wave wave;
    double volume;          /* 0.0 .. 1.0                           */
} Tone;

/* Note frequencies (Hz) used below. */
#define A3  220.00
#define E4  329.63
#define F4  349.23
#define FS4 369.99
#define G4  392.00
#define C5  523.25
#define E5  659.26
#define G5  783.99
#define C6  1046.50
#define E6  1318.51


/* ---- effects as data ----------------------------------------------- */

static const Tone KEY_TONES[] = {
    { 1320, 0, 28, WAVE_PULSE, 0.20 },
};

static const Tone DELETE_TONES[] = {
    { 660, 420, 50, WAVE_SQUARE, 0.16 },
};

static const Tone CLICK_TONES[] = {
    { 880,  0, 30, WAVE_PULSE, 0.22 },
    { 1320, 0, 45, WAVE_PULSE, 0.22 },
};

/* "Wrong": two low buzzes. */
static const Tone INVALID_TONES[] = {
    { 160, 140, 110, WAVE_SQUARE, 0.22 },
    { 0,   0,   50,  WAVE_SQUARE, 0.00 },
    { 150, 115, 170, WAVE_SQUARE, 0.22 },
};

static const Tone TILE_CORRECT_TONES[] = {
    { C6, 0, 110, WAVE_PULSE, 0.22 },
};

static const Tone TILE_PRESENT_TONES[] = {
    { E5, 0, 110, WAVE_PULSE, 0.20 },
};

static const Tone TILE_ABSENT_TONES[] = {
    { A3, 0, 110, WAVE_TRIANGLE, 0.40 },
};

/* Rising arpeggio, then a held high note. */
static const Tone WIN_TONES[] = {
    { C5, 0, 90,  WAVE_PULSE, 0.22 },
    { E5, 0, 90,  WAVE_PULSE, 0.22 },
    { G5, 0, 90,  WAVE_PULSE, 0.22 },
    { C6, 0, 90,  WAVE_PULSE, 0.22 },
    { G5, 0, 90,  WAVE_PULSE, 0.22 },
    { C6, 0, 160, WAVE_PULSE, 0.22 },
    { E6, 0, 420, WAVE_PULSE, 0.20 },
};

/* Four falling notes, the last one sliding down. */
static const Tone LOSE_TONES[] = {
    { G4,  FS4, 220, WAVE_TRIANGLE, 0.45 },
    { FS4, F4,  220, WAVE_TRIANGLE, 0.45 },
    { F4,  E4,  220, WAVE_TRIANGLE, 0.45 },
    { E4,  262, 650, WAVE_TRIANGLE, 0.45 },
};

typedef struct {
    const Tone *tones;
    int count;
} Effect;

#define EFFECT(table) { table, (int)(sizeof(table) / sizeof(table[0])) }

/* Indexed by SoundEffect; the order must match sound.h. */
static const Effect EFFECTS[SOUND_COUNT] = {
    [SOUND_KEY]          = EFFECT(KEY_TONES),
    [SOUND_DELETE]       = EFFECT(DELETE_TONES),
    [SOUND_CLICK]        = EFFECT(CLICK_TONES),
    [SOUND_INVALID]      = EFFECT(INVALID_TONES),
    [SOUND_TILE_CORRECT] = EFFECT(TILE_CORRECT_TONES),
    [SOUND_TILE_PRESENT] = EFFECT(TILE_PRESENT_TONES),
    [SOUND_TILE_ABSENT]  = EFFECT(TILE_ABSENT_TONES),
    [SOUND_WIN]          = EFFECT(WIN_TONES),
    [SOUND_LOSE]         = EFFECT(LOSE_TONES),
};


/* ---- synthesis ----------------------------------------------------- */

static int ms_to_samples(int ms)
{
    return (int)((long)ms * SOUND_SAMPLE_RATE / 1000);
}

/* One waveform value in -1..1 for a phase in 0..1. */
static double wave_value(Wave wave, double phase)
{
    switch (wave) {
    case WAVE_PULSE:    return phase < 0.25 ? 1.0 : -1.0;
    case WAVE_SQUARE:   return phase < 0.50 ? 1.0 : -1.0;
    case WAVE_TRIANGLE: return phase < 0.50 ? 4.0 * phase - 1.0
                                            : 3.0 - 4.0 * phase;
    }
    return 0.0;
}

/*
 * Add one tone into `mix` starting at sample `offset`, `length`
 * samples long. Short fades at both ends prevent audible clicks.
 */
static void add_tone(float *mix, int offset, int length,
                     double frequency, double end_frequency,
                     Wave wave, double volume)
{
    if (frequency <= 0.0 || volume <= 0.0 || length <= 0) {
        return;   /* a rest */
    }

    if (end_frequency <= 0.0) {
        end_frequency = frequency;
    }

    int attack = ms_to_samples(3);
    int release = length / 3;
    double phase = 0.0;

    for (int i = 0; i < length; i++) {
        double progress = (double)i / length;
        double freq = frequency + (end_frequency - frequency) * progress;

        double envelope = 1.0;
        if (i < attack) {
            envelope = (double)i / attack;
        }
        if (i > length - release) {
            envelope *= (double)(length - i) / release;
        }

        mix[offset + i] += (float)(wave_value(wave, phase) * volume * envelope);

        /* Advance the phase by one sample's worth of the wave. */
        phase += freq / SOUND_SAMPLE_RATE;
        if (phase >= 1.0) {
            phase -= 1.0;
        }
    }
}

/* Convert the float mix (-1..1) to 16-bit samples, clamping peaks. */
static short *to_samples(const float *mix, int count)
{
    short *samples = malloc((size_t)count * sizeof(*samples));

    if (samples == NULL) {
        return NULL;
    }

    for (int i = 0; i < count; i++) {
        float v = mix[i];

        if (v > 1.0f)  v = 1.0f;
        if (v < -1.0f) v = -1.0f;

        samples[i] = (short)(v * 32767.0f);
    }

    return samples;
}

int sound_render_effect(SoundEffect effect, short **samples)
{
    if (samples == NULL) {
        return 0;
    }

    *samples = NULL;

    if ((int)effect < 0 || effect >= SOUND_COUNT) {
        return 0;
    }

    const Effect *e = &EFFECTS[effect];
    int total = 0;

    for (int i = 0; i < e->count; i++) {
        total += ms_to_samples(e->tones[i].duration_ms);
    }

    /* calloc returns zeroed memory: silence to add the tones into. */
    float *mix = calloc((size_t)total, sizeof(*mix));
    if (mix == NULL) {
        return 0;
    }

    int offset = 0;

    for (int i = 0; i < e->count; i++) {
        const Tone *t = &e->tones[i];
        int length = ms_to_samples(t->duration_ms);

        add_tone(mix, offset, length, t->frequency, t->end_frequency,
                 t->wave, t->volume);
        offset += length;
    }

    *samples = to_samples(mix, total);
    free(mix);

    return *samples != NULL ? total : 0;
}


/* ---- menu music ---------------------------------------------------- */

/*
 * An original 8-bar tune in C major. Notes are MIDI numbers
 * (60 = middle C, +1 per semitone); 0 is a rest.
 * Melody: eighth notes. Bass: quarter notes.
 */
#define STEP_SAMPLES 5000            /* one eighth note (about 132 BPM) */
#define MELODY_STEPS 64              /* 8 bars x 8 eighths              */
#define BASS_STEPS   32              /* 8 bars x 4 quarters             */

static const unsigned char MELODY[MELODY_STEPS] = {
    72, 76, 79, 76,  72, 76, 79, 84,     /* C  */
    71, 74, 79, 74,  71, 74, 79, 83,     /* G  */
    69, 72, 76, 72,  69, 72, 76, 81,     /* Am */
    65, 69, 72, 69,  67, 71, 74, 79,     /* F  G */
    72, 76, 79, 76,  84, 79, 76, 72,     /* C  */
    74, 77, 81, 77,  74, 77, 81, 86,     /* Dm */
    76, 79, 84, 79,  77, 81, 84, 81,     /* Em F */
    79, 77, 76, 74,  72,  0,  0,  0,     /* G  C */
};

static const unsigned char BASS[BASS_STEPS] = {
    48, 60, 48, 60,
    43, 55, 43, 55,
    45, 57, 45, 57,
    41, 53, 43, 55,
    48, 60, 48, 60,
    50, 62, 50, 62,
    52, 64, 53, 65,
    43, 55, 48, 48,
};

/* MIDI note -> Hz without the maths library: 2^(1/12) per semitone. */
static double note_frequency(int note)
{
    const double semitone = 1.0594630943592953;
    double frequency = 440.0;   /* note 69 = A4 */

    for (int n = 69; n < note; n++) frequency *= semitone;
    for (int n = 69; n > note; n--) frequency /= semitone;

    return frequency;
}

int sound_render_music(short **samples)
{
    if (samples == NULL) {
        return 0;
    }

    int total = MELODY_STEPS * STEP_SAMPLES;
    float *mix = calloc((size_t)total, sizeof(*mix));

    *samples = NULL;

    if (mix == NULL) {
        return 0;
    }

    /* Slightly shorter than a step, so neighbouring notes separate. */
    for (int i = 0; i < MELODY_STEPS; i++) {
        if (MELODY[i] != 0) {
            add_tone(mix, i * STEP_SAMPLES, STEP_SAMPLES * 9 / 10,
                     note_frequency(MELODY[i]), 0, WAVE_PULSE, 0.10);
        }
    }

    for (int i = 0; i < BASS_STEPS; i++) {
        if (BASS[i] != 0) {
            add_tone(mix, i * 2 * STEP_SAMPLES, 2 * STEP_SAMPLES * 9 / 10,
                     note_frequency(BASS[i]), 0, WAVE_TRIANGLE, 0.18);
        }
    }

    *samples = to_samples(mix, total);
    free(mix);

    return *samples != NULL ? total : 0;
}
