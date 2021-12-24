#ifndef GLOBALS_H
#define GLOBALS_H

/*
 * PINS
 * ========================================
 */

/* BUTTON GROUP */
const int buttonGroupButton1Pin = 3,
          buttonGroupButton2Pin = 6,
          buttonGroupButton3Pin = A3,
          buttonGroupButton4Pin = 13;

/* MATRIX */
const int matrixDinPin = 12,
          matrixClockPin = 11,
          matrixLoadPin = 10;

/* JOYSTICK */
const int joystickXPin = A1,
          joystickYPin = A0,
          joystickSWPin = 2;

/* LCD */
const int lcdV0Pin = 9,
          lcdDSPin = 4,
          lcdClockPin = 7,
          lcdLatchPin = 8;

const int lcdShiftRegisterRSPin = 1,
          lcdShiftRegisterEPin = 3,
          lcdShiftRegisterD4Pin = 4,
          lcdShiftRegisterD5Pin = 5,
          lcdShiftRegisterD6Pin = 6,
          lcdShiftRegisterD7Pin = 7;

/* SPEAKER */

const int speakerPin = 5;

/*
 * OTHER CONSTANTS & DEFINES
 * ==========================================
 */

#define SLAVE_NUMBER 7
#define PLAYER_NAME_LENGTH 6
#define DEBOUNCE_DELAY 50
#define NO_NAME "Noname"
#define SCROLL_INTERVAL 500
#define SCROLL_ENABLE_TIMEOUT 3000

/* SETTINGS CONSTANTS */
#define MIN_SLIDER_BLOCK_COUNT 1

#define MIN_CONTRAST 40
#define MAX_CONTRAST 160
#define MAX_CONTRAST_BLOCK_COUNT 12

#define MIN_MAT_BRIGHTNESS 1
#define MAX_MAT_BRIGHTNESS 12
#define MAX_MAT_BRIGHTNESS_BLOCK_COUNT 12

#define MIN_DIFFICULTY 1
#define MAX_DIFFICULTY 3
#define MAX_DIFFICULTY_BLOCK_COUNT 3

/* MENU CONSTANTS */
#define SECTION_COUNT 5
#define MAX_SUBSECTIONS_COUNT 7
#define HIGHSCORE_COUNT 3

#define MAIN_MENU 0
#define PLAY_MENU 1  
#define HIGHSCORE_MENU 2
#define SETTINGS_MENU 3
#define ABOUT_MENU 4

#define ENTER_NAME 1
#define CONTRAST 2
#define MAT_BRIGHTNESS 3
#define DIFFICULTY 4
#define RESET_HIGHSCORE 5

#define SURVIVAL 5

#define MAX_SECTION_LINE_LENGTH 12

/* LCD */
#define DISPLAY_WIDTH 16
#define DISPLAY_HEIGHT 2
#define DISPLAY_PADDED_WIDTH (DISPLAY_WIDTH - 3)

/* MATRIX */
#define MATRIX_HEIGHT 8
#define MATRIX_WIDTH 8
#define MAP_HEIGHT MATRIX_HEIGHT + 1

/* GAME CONSTANTS */
#define MAX_LIVES 30

#define BASE_TONE 400
#define TONE_MULTIPLYER 50
#define TONE_DURATION 50

#define MELODY_REQUEST_COUNT 7
// each melody part consists of note & duration => *2
// note & duration are ints => *(sizeof(int))
// I expect and ending value of -1 / -2 => +sizeof(int)
#define MELODY_BYTES_TO_RECEIVE (MELODY_REQUEST_COUNT * 2 * sizeof(int) + sizeof(int))
#define MELODY_BUFFER_LENGTH (MELODY_REQUEST_COUNT * 2)
#define MELODY_SECTION_END -2
#define MELODY_END -1

#define MELODY_INNITIAL_DELAY 2000
#define BASE_TEMPO 100
#define TEMPO_MULTIPLYER 20
#define WHOLE_NOTE_BAR_COUNT 32

#define NOTE_G4  392
#define NOTE_D5  587
#define CLICK_TONE_DURATION 30

#define ANIMATION_DELAY_SMALL 10
#define ANIMATION_DELAY 50

#endif
