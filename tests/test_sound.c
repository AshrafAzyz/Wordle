/*
 * Unit tests for sound synthesis (src/sound.c).
 *
 * These need no speakers: a sound is just an array of numbers, so
 * we can check its length, loudness, start/end and even its pitch.
 */
#include "sound.h"
#include "test_framework.h"

#include <stdlib.h>
#include <string.h>

#define FULL_SCALE 32767

static int peak(const short *s, int n)
{
    int max = 0;

    for (int i = 0; i < n; i++) {
        int v = s[i] < 0 ? -s[i] : s[i];
        if (v > max) {
            max = v;
        }
    }

    return max;
}

/* Number of times the wave changes sign: two per cycle. */
static int zero_crossings(const short *s, int n)
{
    int count = 0;

    for (int i = 1; i < n; i++) {
        if ((s[i - 1] < 0) != (s[i] < 0)) {
            count++;
        }
    }

    return count;
}

/* Estimated pitch in Hz of a single steady tone. */
static double estimate_frequency(SoundEffect effect)
{
    short *s = NULL;
    int n = sound_render_effect(effect, &s);
    double seconds = (double)n / SOUND_SAMPLE_RATE;
    double hz = n > 0 ? zero_crossings(s, n) / 2.0 / seconds : 0.0;

    free(s);

    return hz;
}

static int within(double value, double target, double tolerance)
{
    return value > target * (1.0 - tolerance) &&
           value < target * (1.0 + tolerance);
}


static void test_every_effect_renders(void)
{
    for (int e = 0; e < SOUND_COUNT; e++) {
        short *s = NULL;
        int n = sound_render_effect((SoundEffect)e, &s);
        int ms = n * 1000 / SOUND_SAMPLE_RATE;

        if (s == NULL || n <= 0) {
            printf("    effect %d did not render\n", e);
            CHECK(0);
            continue;
        }

        CHECK(ms >= 20 && ms <= 2000);            /* sensible length      */
        CHECK(peak(s, n) > FULL_SCALE / 20);      /* audible              */
        CHECK(peak(s, n) < FULL_SCALE * 6 / 10);  /* room to mix, no clip */

        /* Fades in and out, so playing it never makes a click. */
        CHECK(abs(s[0]) < 500);
        CHECK(abs(s[n - 1]) < 500);

        free(s);
    }
}

static void test_rendering_is_deterministic(void)
{
    for (int e = 0; e < SOUND_COUNT; e++) {
        short *a = NULL;
        short *b = NULL;
        int na = sound_render_effect((SoundEffect)e, &a);
        int nb = sound_render_effect((SoundEffect)e, &b);

        CHECK(na == nb);
        CHECK(na > 0 && memcmp(a, b, (size_t)na * sizeof(short)) == 0);

        free(a);
        free(b);
    }
}

static void test_invalid_requests_fail_safely(void)
{
    short *s = (short *)&s;   /* any non-NULL value */

    CHECK(sound_render_effect(SOUND_COUNT, &s) == 0);
    CHECK(s == NULL);
    CHECK(sound_render_effect((SoundEffect)-1, &s) == 0);
    CHECK(sound_render_effect(SOUND_KEY, NULL) == 0);
    CHECK(sound_render_music(NULL) == 0);
}

/* Green, yellow and grey tiles are told apart by pitch. */
static void test_tile_notes_have_the_right_pitch(void)
{
    double correct = estimate_frequency(SOUND_TILE_CORRECT);
    double present = estimate_frequency(SOUND_TILE_PRESENT);
    double absent  = estimate_frequency(SOUND_TILE_ABSENT);

    CHECK(within(correct, 1046.5, 0.10));   /* C6 */
    CHECK(within(present, 659.3, 0.10));    /* E5 */
    CHECK(within(absent, 220.0, 0.10));     /* A3 */
    CHECK(correct > present && present > absent);
}

static void test_music_loops_cleanly(void)
{
    short *s = NULL;
    int n = sound_render_music(&s);

    CHECK(s != NULL);
    CHECK(n == 64 * 5000);                    /* 8 bars of 8 eighths   */

    if (s != NULL && n > 0) {
        CHECK(peak(s, n) > FULL_SCALE / 20);   /* audible               */
        CHECK(peak(s, n) < FULL_SCALE * 4 / 10); /* quieter than effects */

        /* End and start are silent, so the loop joins without a click. */
        CHECK(abs(s[0]) < 500);
        CHECK(abs(s[n - 1]) < 500);
    }

    free(s);
}


int main(void)
{
    printf("=== sound tests ===\n");

    RUN_TEST(test_every_effect_renders);
    RUN_TEST(test_rendering_is_deterministic);
    RUN_TEST(test_invalid_requests_fail_safely);
    RUN_TEST(test_tile_notes_have_the_right_pitch);
    RUN_TEST(test_music_loops_cleanly);

    return TEST_SUMMARY();
}
