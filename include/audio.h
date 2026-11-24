#ifndef AUDIO_H
#define AUDIO_H

#include "sound.h"

/*
 * Audio playback through ALSA (which PipeWire/PulseAudio route to
 * the speakers). A background thread mixes the menu music and any
 * number of overlapping effects, so playing a sound never blocks
 * the game.
 *
 * If no audio device can be opened, or the game was built with
 * `make NO_SOUND=1`, every function still works but does nothing.
 */

/* Start audio. Returns 1 if sound works, 0 if the game is silent. */
int audio_init(void);

/* Stop the audio thread and free everything. Safe to call twice. */
void audio_shutdown(void);

/* Play an effect now, or after delay_ms milliseconds. */
void audio_play(SoundEffect effect);
void audio_play_after(SoundEffect effect, int delay_ms);

/* Start (1) or stop (0) the looping menu music, with a short fade. */
void audio_set_music(int playing);

/* Mute switch for everything (effects and music). */
void audio_set_muted(int muted);
int audio_is_muted(void);

/* 1 if a sound device is open. */
int audio_is_available(void);

#endif
