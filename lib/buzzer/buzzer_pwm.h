#ifndef BUZZER_PWM_H
#define BUZZER_PWM_H

#include <stdint.h>
#include <stdbool.h>

// Common musical note frequencies (in Hz)
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_B5  988
#define NOTE_C6  1047

// Common beep frequencies
#define FREQ_LOW     500   // Low beep
#define FREQ_MEDIUM  1000  // Medium beep
#define FREQ_HIGH    2000  // High beep
#define FREQ_ALARM   2500  // Alarm sound

/**
 * Initialize PWM buzzer on a specific GPIO pin
 * 
 * @param pin GPIO pin number for the buzzer
 */
void buzzer_pwm_init(uint pin);

/**
 * Play a tone at a specific frequency
 * 
 * @param frequency Frequency in Hz (e.g., 1000 = 1kHz)
 * @param duration_ms Duration in milliseconds (0 = continuous)
 */
void buzzer_play_tone(uint32_t frequency, uint32_t duration_ms);

/**
 * Stop the buzzer (silence)
 */
void buzzer_stop(void);

/**
 * Play a beep at specified frequency
 * 
 * @param frequency Frequency in Hz
 * @param duration_ms Duration of the beep
 */
void buzzer_beep(uint32_t frequency, uint32_t duration_ms);

/**
 * Play a musical note
 * 
 * @param note Note frequency (use NOTE_* defines)
 * @param duration_ms Duration in milliseconds
 */
void buzzer_play_note(uint32_t note, uint32_t duration_ms);

/**
 * Play a simple melody (array of frequencies and durations)
 * 
 * @param frequencies Array of frequencies in Hz
 * @param durations Array of durations in ms
 * @param note_count Number of notes to play
 */
void buzzer_play_melody(const uint32_t *frequencies, const uint32_t *durations, uint note_count);

/**
 * Set PWM duty cycle (volume control)
 * 
 * @param duty Duty cycle percentage (0-100)
 */
void buzzer_set_volume(uint8_t duty);

#endif // BUZZER_PWM_H