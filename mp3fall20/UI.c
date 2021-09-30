/*
 * UI.c
 *
 *  Created on:  Nov 1, 2018
 *  Modified on: Aug 9, 2019
 *  Modified on: Oct 27, 2020
 *      Author: khughes
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Peripherals
#include "osc.h"
#include "gptm.h"
#include "gpio.h"
#include "ppb.h"
#include "sysctl.h"
#include "keypad.h"
#include "lookup.h"
#include "sound.h"
#include "lcd.h"

// Various hooks for MP3 player
#include "sound.h"
#include "control.h"
#include "UI.h"

// Definitions for function keys
enum keycmds_t {
  PLAY_PAUSE    = 'A',
  SHUFFLE       = 'B',
  VOLUME_UP     = 'C',
  VOLUME_DOWN   = 'D',
  SKIP_BACKWARD = 'E',
  SKIP_FORWARD  = 'F',
};

// Song Info string
static char* songstr;
// Temp pointer to change start of songstr to have scrolling text
static uint8_t offset = 0;
// Temp pointer to start of next line to have scrolling text
static uint8_t last = 20;

// Song Info string
static uint8_t wave[15] = {0};

// Timer to keep track of elapsed time
static uint16_t timer = 0;

static uint8_t v = 0;

// Define keyboard information for Jose Jimenez's Keyboard
#ifdef JOSE
// Your keypad key assignments from Lab 4.
static const uint8_t keymap[4][4] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'E','0','F','D'},
};

// Your keypad pin assignments from Lab 4.
const struct portinfo rowdef = {
    {GPIO_PIN_3, GPIO_PIN_2, GPIO_PIN_1, GPIO_PIN_0},
    GPIO_PORTH,
    //{GPIO_PIN_5, GPIO_PIN_4, GPIO_PIN_3, GPIO_PIN_2},
    //GPIO_PORTP,
};

const struct portinfo columndef = {
    {GPIO_PIN_4, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7},
    GPIO_PORTC,
    //{GPIO_PIN_2, GPIO_PIN_3, GPIO_PIN_4, GPIO_PIN_5},
    //GPIO_PORTA,
};
#endif

// Define keyboard information for Ellis Chen's Keyboard
#ifdef ELLIS
// Your keypad key assignments from Lab 4.
static const uint8_t keymap[4][4] = {
    {'E','0','F','D'},
    {'7','8','9','C'},
    {'4','5','6','B'},
    {'1','2','3','A'},
};

// Your keypad pin assignments from Lab 4.
const struct portinfo rowdef = {
    {GPIO_PIN_3, GPIO_PIN_2, GPIO_PIN_1, GPIO_PIN_0},
    GPIO_PORTH,
};

const struct portinfo columndef = {
    {GPIO_PIN_4, GPIO_PIN_3, GPIO_PIN_2, GPIO_PIN_1},
    GPIO_PORTL,
};
#endif

// Define keyboard information for Zach Kingsland's Keyboard
#ifdef ZACH
// Your keypad key assignments from Lab 4.
static const uint8_t keymap[4][4] = {
};

// Your keypad pin assignments from Lab 4.
const struct portinfo rowdef = {
};

const struct portinfo columndef = {
};
#endif

// Return key value upon initial key press.  Since keys are
// ASCII, they cannot have a value greater than 0xff, so when
// no new key is pressed the procedure returns UINT16_MAX.
static uint16_t UIKey( void ) { 
  static enum {NOT_PRESSED, PRESSED} state = NOT_PRESSED;
  uint8_t column, row, key;

#define ADDR keymap
#define SIZE 4

  switch( state ) {
  case NOT_PRESSED:
    if( getKey( &column, &row ) == true ) {
      key = LOOKUP();
      state = PRESSED;
      return (uint16_t)key;
    }
    break;
  case PRESSED:
    if( getKey( &column, &row ) == false )
      state = NOT_PRESSED;
    break;
  }
  return UINT16_MAX;
}

static void selectMode(uint16_t firstkey){

    uint8_t userCmd = (firstkey - 48); // convert to int value

    while(true){
      uint16_t key = UIKey( );
      if( key != UINT16_MAX ) {
          if((enum keycmds_t)key == 'E'){   // 'E' Enter key

              // If command entered is valid set the song to that ID, else return to play mode
              if((0 < userCmd) && (userCmd < (getNumSongs()+2))){

                  // Flag to play this song now
                  setUserChose(true);

                  // Set user defined song
                  setSong(userCmd-1);

                  // Set current song done
                  setDone();

                  // Unpause and break
                  setPaused(false);

                  break;
              }else{

                  // Invalid song ID, unpause and break
                  setPaused(false);
                  break;
              }
          }else if((enum keycmds_t)key == 'C'){ // 'C' Clear key

              // Clear userCmd variable
              userCmd = 0;


          }

          /* I set it up so that when 'A' is pushed, then the song is liked and added to separate play list
           * If 'A' is pushed and song already is liked, then the song will be disliked and removed from list
           * When 'B' is pushed, songs will start playing from play list
           * When 'B' is pushed again, songs will resume back to normal
           * The play list is NOT COMPATIBLE with shuffle mode
           * DID NOT PROGRAM TO DISPLAY ANYTHING WHEN LIKED OR DISLIKED
           * NOR PROGRAM DISPLAY FOR INDICATING MODE BETWEEN SEPARATE (LIKED) PLAY LIST OR NORMAL LIST
           */

            else if((enum keycmds_t)key == 'A'){ // 'A' Like and save to separate play list
                if(getDone() == false) {
                    v = readSeparateList();
                    if (v < 255) { // if v < 255, then the current song already exists in list
                        // if song already exists, will dislike and remove from list
                        deleteSeparateList( v );
                        break;
                    }
                    else { // will like and store current song into separate play list
                        storeSeparateList();
                         break;
                    }
                }
            }
            else if((enum keycmds_t)key == 'B') {
                //isListMode = checkListMode(); // checks if we are in the separate play list or not
                setListMode( isListMode() == false );
                if (getDone() == false) {
                    setDone(); // finish current song
                    setPaused( isPaused() == false );
                    setTimer(getTimer()-1);
                    updateUI();

                    break;
                }
            }

          else{    // Other keys pressed

              // If key is valid number key, then concatenate to end of userCmd
              if(key >= '0' && key <= '9'){

                  // Concatenate to end of userCmd
                  userCmd = (userCmd * 10) + (key - 48);
              }
          }
      }
    }
}

// Interrupt handler for user interface.  When called it scans the
// keypad and if a new key is detected, performs necessary actions.
void UIHandler( void ) {
  uint16_t key = UIKey( ); 
  if( key != UINT16_MAX ) {
    switch( (enum keycmds_t)key ) {
    case PLAY_PAUSE:    // 'A'
      setPaused( isPaused() == false );
      setTimer(getTimer()-1);
      updateUI();
      break;
    case SHUFFLE:       // 'B'
      setShuffle( isShuffle() == false );
      break;
    case VOLUME_UP:     // 'C'
      upVolume();
      break;
    case VOLUME_DOWN:   // 'D'
      downVolume();
      break;
    case SKIP_BACKWARD: // 'E'
      setSkipBack();
      if (isDone() == false) {
        setDone();
      }
      break;
    case SKIP_FORWARD:  // 'F'
      setDone();
      break;
    default:            // Numeric keys

      // Check if in pause mode
      if(isPaused()){
          // Go into select mode
          selectMode(key);
      }

      break;
    }
  }

  // Clear the time-out.
  GPTM_TIMER5[GPTM_ICR] |= GPTM_ICR_TATOCINT;
}

void initUI( void ) {
  // Prep the keypad.
  initKeypad ( &columndef, &rowdef );

  // Enable Run Clock Gate Control
  SYSCTL[SYSCTL_RCGCTIMER] |= SYSCTL_RCGCTIMER_TIMER5;
  SYSCTL[SYSCTL_RCGCTIMER] |= SYSCTL_RCGCTIMER_TIMER5;

  // 32-bit periodic timer.
  GPTM_TIMER5[GPTM_CFG] &= ~GPTM_CFG_M;
  GPTM_TIMER5[GPTM_CFG] |= GPTM_CFG_32BITTIMER;
  GPTM_TIMER5[GPTM_TAMR] &= ~GPTM_TAMR_TAMR_M;
  GPTM_TIMER5[GPTM_TAMR] |= GPTM_TAMR_TAMR_PERIODIC;

  // Set reload value for 20Hz
  GPTM_TIMER5[GPTM_TAILR] = (MAINOSCFREQ/20)-1;

  // Enable the interrupt.
  GPTM_TIMER5[GPTM_IMR] |= GPTM_IMR_TATOIM;

  // Enable interrupt for timer.
  PPB[PPB_EN2] |= PPB_EN2_TIMER5A;

  // Set priority level to 1 (lower priority than Timer2A).
  PPB[PPB_PRI16] = ( PPB[PPB_PRI16] & PPB_PRI_INTB_M ) | ( 1 << PPB_PRI_INTB_S );

  // Clear the time-out.
  GPTM_TIMER5[GPTM_ICR] |= GPTM_ICR_TATOCINT;

  // Enable the timer.
  GPTM_TIMER5[GPTM_CTL] |= GPTM_CTL_TAEN | GPTM_CTL_TASTALL;
}

static void drawStatusInfo(void){
    // TEMPORARY: ZACK NEEDS TO IMPLEMENT LOGIC

    // ascii status info
    char status_info[21] = {0};

    // Hashtag (Pound sign)
    strcat(status_info, "#");

    // Get song number
    uint8_t currentSong = getCurrentSong();

    // Display extra zero in front if necessary
    if((currentSong + 1) < 10){
        strcat(status_info, "0");
    }

    // Convert song num to str
    char str[2];
    sprintf(str, "%u", currentSong+1);

    // Display song numer
    strcat(status_info, str);

    // Space
    strcat(status_info, " ");

    // Display pause or unpause icon
    if(isPaused()){

        // If paused, display pause icon
        strcat(status_info, "‚");
    }else{

        // If not paused, diplay play icon
        strcat(status_info, "€");
    }

    // Spacing
    strcat(status_info, "   ");

    // Calculate time elapsed
    uint8_t timer_min = timer / 60;
    uint8_t timer_sec = timer % 60;

    // Display extra zero in front if necessary
    if((timer_min) < 10){
        strcat(status_info, "0");
    }

    // Convert minutes to str
    sprintf(str, "%u", timer_min);

    // Display minutes
    strcat(status_info, str);

    // Minutes and seconds separator
    strcat(status_info, ":");

    // Display extra zero in front if necessary
    if((timer_sec) < 10){
        strcat(status_info, "0");
    }

    // Convert seconds to str
    sprintf(str, "%u", timer_sec);

    // Display seconds
    strcat(status_info, str);

    // Increment Timer
    timer++;

    // Display Speaker & sound icon
    strcat(status_info, "   ƒ„ ");

    // Get Volume
    uint8_t vol = getVolume();

    // Convert volume to str
    sprintf(str, "%u", vol);

    // Display extra zero in front if necessary
    if((vol) < 10){
        strcat(status_info, "0");
    }

    // Display volume
    strcat(status_info, str);

    // Set position
    positionLCD(0, 0);

    // Print to LCD
    stringLCD((uint8_t*)status_info);
}

static void drawWaveform(void){

    // Waveform icons with differing levels of "amplitude"
    char waveLevels[9][2] = {
        {' ', 0}, // 0-level wavelet
        {'Œ', 0}, // 1-level wavelet
        {'‹', 0}, // 2-level wavelet
        {'Š', 0}, // 3-level wavelet
        {'‰', 0}, // 4-level wavelet
        {'ˆ', 0}, // 5-level wavelet
        {'‡', 0}, // 6-level wavelet
        {'†', 0}, // 7-level wavelet
        {'…', 0}, // 8-level wavelet
    };

    // ascii status info
    char wave_form[3][21] = {0};

    // Col update
    for(int j = 0; j < 15; j++){

        // Row update
        for(int i = 2; i < 6; i++){

            // Check if element from wave array is not zero
            if(wave[j] != 0){

                for(int k = 8; k <= 64; k+=8){

                    // Check if blank
                    if(wave[j] < ((64*(6-i))-64)){
                        // Display blank
                        strcat(wave_form[i-3], waveLevels[0]);

                        // Break from for loop
                        break;
                    }

                    // Translate sound data to icon data
                    if(wave[j] >= ((64*(6-i))-k)){

                        // Display volume
                        strcat(wave_form[i-3], waveLevels[9-(k/8)]);

                        // Break from for loop
                        break;
                    }
                }
            }
        }
    }

    for(int ii = 3; ii < 6; ii++){
        // Set position
        positionLCD(ii, 3);

        // Print to LCD
        stringLCD((uint8_t*)wave_form[ii-3]);
    }
}

static void drawBorder(void){

    // ascii border character srtings
    char* upper_border = "_____________________";
    char* lower_border = "_____________________";

    // Set position
    positionLCD(1, 0);

    // Print to LCD
    stringLCD((uint8_t*)upper_border);

    // Set position
    positionLCD(6, 0);

    // Print to LCD
    stringLCD((uint8_t*)lower_border);
}

static void drawContinuousScrolling(void){
    // Continuous scrolling logic
    if(*(songstr+offset+last) == 0){

        // Set position
        positionLCD(7, 0);

        // Print to LCD the song info
        stringLCD((uint8_t*)(songstr+offset));

        // Check if loop around is needed
        if(*(songstr+offset) != 0){

            // Increase offset
            offset++;
        }else{

            // Reset to beginning
            offset = 1;
        }

        // Set position
        positionLCD(7, last);

        // Print the start of the next line
        stringLCD((uint8_t*)(songstr));

        // Check if loop around is needed
        if(last != 0){

            // Increase offset
            last--;
        }else{

            // Reset to end
            last = 20;
        }

    }else{

        // Set position
        positionLCD(7, 0);

        // Print to LCD the song info
        stringLCD((uint8_t*)(songstr+offset));

        // Check if loop around is needed
        if(*(songstr+offset) != 0){

            // Increase offset
            offset++;
        }else{

            // Reset to beginning
            offset = 0;
        }
    }
}

// Set song info string
void setSongInfoStr(char* str){
    songstr = str;
    offset = 0;
}

// Get Timer value
uint16_t getTimer(void){
    return timer;
}

// Set Timer Value
void setTimer(uint16_t t){
    timer = t;
}

// Add sound sample to waveform array
void addWavelet(uint8_t w){

    // Shift wave array left by 1 element
    for(int i = 0; i < 14; i++){
        wave[i] = wave[i+1];
    }

    // Add sound sample
    wave[14] = w;
}

// Reset waveform
void resetWavelet(void){
    for(int i = 0; i < 15; i++){
        wave[i] = 0;
    }
}

// Update UI procedure
void updateUI(void){

    // Clear LCD
    clearLCD();

    // Draw status info bar
    drawStatusInfo();

    // Draw waveform
    drawWaveform();

    // Draw top and bottom border
    drawBorder();

    // Draw continuous scrolling song info
    drawContinuousScrolling();
}
