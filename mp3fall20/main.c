/*
 * lab08.c
 *
 *  Created on:  Aug 13, 2018
 *  Modified on: Jul 19, 2019
 *  Modified on: May 29, 2020
 *      Author: khughes
 */

#include <ssi1_DAC.h>
#include <stdint.h>
#include <stdbool.h>
#include <timer2A.h>
#include <string.h>

// Peripherals
#include "osc.h"
#include "ssi_uSD.h"
#include "lcd.h"
#include "ff.h"

// Utilities
#include "MP3utils.h"
#include "ID3.h"

// Other project files
#include "mp3.h"
#include "sound.h"
#include "control.h"
#include "UI.h"

// ID3 tags
struct id3tags tags;

// Decode and play an MP3 file.
void playSong( uint8_t song  ) {
    FIL fp;

    // Get the file from the MicroSD card.
    findMP3( song, &fp );

    // Process ID3 header (if any).
    getID3Tags( &fp , &tags );

    // Prepare for sound output.
    initSound();

    // Create song info
    char title_info[200] = {0};
    strcat(title_info, "Title: ");
    strcat(title_info, (char*)tags.title);
    strcat(title_info, "  ----  ");
    strcat(title_info, "Artist: ");
    strcat(title_info, (char*)tags.artist);
    strcat(title_info, "  ----  ");
    strcat(title_info, "Album: ");
    strcat(title_info, (char*)tags.album);
    strcat(title_info, "  ----  ");

    // Pass string to UI functions
    setSongInfoStr(title_info);

    // Decode and play the song.
    MP3decode( &fp );

    // Wait for the last data to be sent to the DACs.
    while( isSoundFinished() == false ) __asm( " wfi" );

    // Stop the DAC timer.
    enableTimer2A( false );
}

main() {
  // Initialize clock, SSIs, and Timer
  initOsc();
  initSSI3();
  initLCD( false );
  initSSI1();
  initTimer2A();
  initUI();

  // Initialize structure.
  FATFS fs; f_mount( &fs, "", 0 );

  // Find out how many MP3 files are on the card.
  uint8_t numSongs = countMP3();
  setNumSongs( numSongs );

  // Obligatory endless loop.
  while( true ) {
    // Get the next file from the MicroSD card.
    uint8_t song = getSong();

    // Send the file to the MP3 decoder
    playSong( song );
  }
}
