/*
 * Audio playback (ALSA) with a background mixing thread.
 *
 * Threads and locking
 * -------------------
 * The game (main thread) and the mixer (audio thread) share the
 * `voices` list and the music settings. Every access to them is
 * wrapped in pthread_mutex_lock/unlock, so the two threads never
 * see half-updated data. The mixer holds the lock only while
 * mixing one short block, never while waiting for the sound card.
 *
 * Build without sound: `make NO_SOUND=1` defines WORDLE_NO_SOUND,
 * which compiles the small silent versions at the bottom instead.
 */

#ifndef WORDLE_NO_SOUND

/* For POSIX types used by the ALSA headers. Must precede #includes. */
#define _POSIX_C_SOURCE 200809L

#include "audio.h"

#include <alsa/asoundlib.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VOICES     16      /* effects that can overlap            */
#define BLOCK_FRAMES   256     /* samples mixed per loop (~12 ms)     */
#define LATENCY_US     100000  /* ALSA buffer: 100 ms (slack for VMs) */
#define FADE_SAMPLES   (SOUND_SAMPLE_RATE / 4)   /* music fade: 0.25 s */
#define WARMUP_SAMPLES SOUND_SAMPLE_RATE         /* 1 s before music  */

typedef struct {
    const short *data;     /* NULL = free slot                 */
    int length;
    int position;
    int delay;             /* samples to wait before starting  */
} Voice;

static struct {
    int available;
    snd_pcm_t *pcm;
    pthread_t thread;
    pthread_mutex_t lock;

    /* Shared with the audio thread: only touch while holding `lock`. */
    int running;
    int muted;
    int music_wanted;
    int warmup;            /* samples of silence left before music */
    int music_level;       /* 0..FADE_SAMPLES: current music volume */
    int music_position;
    Voice voices[MAX_VOICES];

    /* Written once in audio_init(), then only read. */
    short *effects[SOUND_COUNT];
    int effect_length[SOUND_COUNT];
    short *music;
    int music_length;
} audio;


/* ---- the audio thread ---------------------------------------------- */

/* Mix one block of output. Caller holds the lock. */
static void mix_block(short *out, int frames)
{
    int mix[BLOCK_FRAMES];

    memset(mix, 0, sizeof(mix));

    /* Effects. */
    for (int v = 0; v < MAX_VOICES; v++) {
        Voice *voice = &audio.voices[v];

        if (voice->data == NULL) {
            continue;
        }

        for (int i = 0; i < frames; i++) {
            if (voice->delay > 0) {
                voice->delay--;
                continue;
            }
            if (voice->position >= voice->length) {
                voice->data = NULL;   /* finished: free the slot */
                break;
            }
            mix[i] += voice->data[voice->position++];
        }
    }

    /* Music, faded in/out smoothly instead of starting abruptly. */
    for (int i = 0; i < frames; i++) {
        /*
         * Hold the music back for the first second: the sound card is
         * often waking from power-saving and may stutter briefly, and
         * gaps in silence cannot be heard. Effects are not delayed.
         */
        if (audio.warmup > 0) {
            audio.warmup--;
        }
        else if (audio.music_wanted && audio.music_level < FADE_SAMPLES) {
            audio.music_level++;
        }
        else if (!audio.music_wanted && audio.music_level > 0) {
            audio.music_level--;
        }

        if (audio.music_level > 0 && audio.music != NULL) {
            mix[i] += (int)((long)audio.music[audio.music_position] *
                            audio.music_level / FADE_SAMPLES);
            audio.music_position = (audio.music_position + 1) % audio.music_length;
        }
        else if (!audio.music_wanted) {
            audio.music_position = 0;   /* restart the tune next time */
        }
    }

    /* Clamp the sum into the 16-bit range (prevents wrap-around noise). */
    for (int i = 0; i < frames; i++) {
        int value = audio.muted ? 0 : mix[i];

        if (value > 32767)  value = 32767;
        if (value < -32768) value = -32768;

        out[i] = (short)value;
    }
}

static void *audio_thread(void *unused)
{
    (void)unused;
    short block[BLOCK_FRAMES];

    for (;;) {
        pthread_mutex_lock(&audio.lock);
        int running = audio.running;
        if (running) {
            mix_block(block, BLOCK_FRAMES);
        }
        pthread_mutex_unlock(&audio.lock);

        if (!running) {
            break;
        }

        /* Hand the block to ALSA; wait (max 100 ms) while its buffer is full. */
        int written = 0;

        while (written < BLOCK_FRAMES) {
            snd_pcm_sframes_t n =
                snd_pcm_writei(audio.pcm, block + written,
                               (snd_pcm_uframes_t)(BLOCK_FRAMES - written));

            if (n == -EAGAIN) {
                snd_pcm_wait(audio.pcm, 100);
                continue;
            }
            if (n < 0) {
                /* Underrun or suspend: let ALSA recover, else drop the block. */
                if (snd_pcm_recover(audio.pcm, (int)n, 1) < 0) {
                    break;
                }
                continue;
            }

            written += (int)n;
        }
    }

    return NULL;
}


/* ---- public API ---------------------------------------------------- */

static void free_sounds(void)
{
    for (int i = 0; i < SOUND_COUNT; i++) {
        free(audio.effects[i]);
        audio.effects[i] = NULL;
    }

    free(audio.music);
    audio.music = NULL;
}

int audio_init(void)
{
    if (audio.available) {
        return 1;
    }

    /* Render every sound once, up front. */
    for (int i = 0; i < SOUND_COUNT; i++) {
        audio.effect_length[i] = sound_render_effect((SoundEffect)i, &audio.effects[i]);
    }
    audio.music_length = sound_render_music(&audio.music);

    /* Non-blocking, so the thread can notice shutdown while waiting. */
    int err = snd_pcm_open(&audio.pcm, "default",
                           SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);

    if (err >= 0) {
        err = snd_pcm_set_params(audio.pcm, SND_PCM_FORMAT_S16,
                                 SND_PCM_ACCESS_RW_INTERLEAVED,
                                 1, SOUND_SAMPLE_RATE, 1, LATENCY_US);
        if (err < 0) {
            snd_pcm_close(audio.pcm);
        }
    }

    if (err < 0) {
        fprintf(stderr, "Sound disabled: %s\n", snd_strerror(err));
        audio.pcm = NULL;
        free_sounds();
        snd_config_update_free_global();
        return 0;
    }

    pthread_mutex_init(&audio.lock, NULL);
    audio.running = 1;
    audio.warmup = WARMUP_SAMPLES;

    if (pthread_create(&audio.thread, NULL, audio_thread, NULL) != 0) {
        fprintf(stderr, "Sound disabled: could not start audio thread\n");
        pthread_mutex_destroy(&audio.lock);
        snd_pcm_close(audio.pcm);
        audio.pcm = NULL;
        free_sounds();
        snd_config_update_free_global();
        return 0;
    }

    audio.available = 1;
    return 1;
}

void audio_shutdown(void)
{
    if (!audio.available) {
        return;
    }

    pthread_mutex_lock(&audio.lock);
    audio.running = 0;
    pthread_mutex_unlock(&audio.lock);

    /* Wait for the thread to finish its last block. */
    pthread_join(audio.thread, NULL);

    snd_pcm_drop(audio.pcm);
    snd_pcm_close(audio.pcm);
    audio.pcm = NULL;

    /* ALSA caches its configuration globally; release it too. */
    snd_config_update_free_global();

    pthread_mutex_destroy(&audio.lock);
    free_sounds();
    memset(audio.voices, 0, sizeof(audio.voices));
    audio.available = 0;
}

void audio_play_after(SoundEffect effect, int delay_ms)
{
    if (!audio.available || (int)effect < 0 || effect >= SOUND_COUNT ||
        audio.effects[effect] == NULL) {
        return;
    }

    pthread_mutex_lock(&audio.lock);

    /* Use a free slot; if all 16 are busy, replace the oldest-started. */
    int slot = 0;
    for (int v = 0; v < MAX_VOICES; v++) {
        if (audio.voices[v].data == NULL) {
            slot = v;
            break;
        }
        if (audio.voices[v].position > audio.voices[slot].position) {
            slot = v;
        }
    }

    audio.voices[slot].data = audio.effects[effect];
    audio.voices[slot].length = audio.effect_length[effect];
    audio.voices[slot].position = 0;
    audio.voices[slot].delay =
        (int)((long)(delay_ms > 0 ? delay_ms : 0) * SOUND_SAMPLE_RATE / 1000);

    pthread_mutex_unlock(&audio.lock);
}

void audio_play(SoundEffect effect)
{
    audio_play_after(effect, 0);
}

void audio_set_music(int playing)
{
    if (!audio.available) {
        return;
    }

    pthread_mutex_lock(&audio.lock);
    audio.music_wanted = playing ? 1 : 0;
    pthread_mutex_unlock(&audio.lock);
}

void audio_set_muted(int muted)
{
    if (!audio.available) {
        return;
    }

    pthread_mutex_lock(&audio.lock);
    audio.muted = muted ? 1 : 0;
    pthread_mutex_unlock(&audio.lock);
}

int audio_is_muted(void)
{
    if (!audio.available) {
        return 1;
    }

    pthread_mutex_lock(&audio.lock);
    int muted = audio.muted;
    pthread_mutex_unlock(&audio.lock);

    return muted;
}

int audio_is_available(void)
{
    return audio.available;
}


#else  /* WORDLE_NO_SOUND: silent versions, no ALSA needed */

#include "audio.h"

int audio_init(void)                              { return 0; }
void audio_shutdown(void)                         { }
void audio_play(SoundEffect effect)               { (void)effect; }
void audio_play_after(SoundEffect effect, int ms) { (void)effect; (void)ms; }
void audio_set_music(int playing)                 { (void)playing; }
void audio_set_muted(int muted)                   { (void)muted; }
int audio_is_muted(void)                          { return 1; }
int audio_is_available(void)                      { return 0; }

#endif
