/*
 * control.c
 *
 *  Created on:  Aug 13, 2018
 *  Modified on: May 29, 2020
 *      Author: khughes
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "control.h"
#include "UI.h"

// The total number of songs on the MicroSD card.
static uint8_t numSongs;
// The next song to play.
static uint8_t song = 0;
//
static uint8_t nextFavSong = 0;
// Indicates termination of current song by user.
static bool done = false;
// Indicates whether decoding/playing is playing or paused.
static bool paused = false;
// Indicates if the MP3 player is in song shuffle mode.
static bool shuffle = false;
// Indicates if the MP3 player plays the previous song.
static bool skipBack = false;
// Indicates if songs need to be shuffled
static bool initialize = false;
// Stores an array to keep track of the songs in shuffle mode
static uint8_t* shuffledSongs;
// Initialize song choice
static bool initial = false;
// Indicates whether song was picked by user, so play that song
static bool userChose = false;

// Stores separate songs in new play list
static uint8_t separateList[10] = {0};
// This will only be true once; sets flag to initialize separate play list
static bool initList = true;
// Indicates if we are playing songs from separate list
static bool listMode = false;

// Private procedure for selecting the next song in shuffle mode.
static uint8_t getShuffle( uint8_t song ) {
    if (initialize == true) {
        initializeShuffle();
        initialize = false;
    }
    else {
        if (skipBack == false) {
            song = shuffledSongs[song] + 1;
        }
        else {
            song = shuffledSongs[song] - 1;
            skipBack = false;
        }
    }
  return song;
}

/*Seems like program doesn't compile when static functions are not declared in the beginning*/
/* private function to get the play the next song from separate play list*/
static uint8_t getSongFromList( uint8_t song ) {
    if (skipBack == false) {
        song = separateList[nextFavSong];
        nextFavSong++;

        if(nextFavSong == 10 || song == 255){
            nextFavSong = 0;
        }
    }
    else {
        song = separateList[nextFavSong];
        nextFavSong--;

        if(nextFavSong == 255 || song == 255){
            nextFavSong = 9;
        }
        skipBack = false;
    }
    return song;
}

void initializeShuffle( void ) {
    shuffledSongs = (uint8_t *)malloc(numSongs * sizeof(uint8_t));
    for (uint8_t i = 0; i < numSongs; i++) {
        shuffledSongs[i] = (rand() % (numSongs + 1));
    }
}

// Return the number of the song to play.  Initially, just
// return the next sequential song, and wrap back to 0 when all
// songs have been returned.
uint8_t getSong( void ) {

  // On the first call, always return the first song.
  if( initial == false ) {
    initial = true;
    return song = 0;
  }

  // If user selected song, play that over any shuffle or serial mode
  if(userChose == true){
      userChose = false;
      return song;
  }

  if( listMode == true) {
        song = getSongFromList( song ); // returns songs from the separate play list
        return song;
    }

  // Otherwise pick the next song to play.
  if( shuffle == false ) {
      if (skipBack == false) {
    song = ( song + 1 ) % numSongs;
      }
      else {
          song = ( song - 1 ) % numSongs;
          skipBack = false;
      }
  } else {
    song = getShuffle( song );
  }

  setTimer(0);

  // Return song number.
  return song;
}

// Store the total number of songs (needed by getSong()).
void setNumSongs( uint8_t num ) {
  numSongs = num;
}

// Store the total number of songs (needed by getSong()).
uint8_t getNumSongs( void ) {
  return numSongs;
}

// Set next song explicitly.
void setSong( uint8_t v ) {
  song = v;
  setTimer(0);
}

// Indicates whether the current MP3 song should be terminated early.
bool isDone( void ) {
  bool tmp = done;
  done = false;
  return tmp;
}

// Get if song is done playing
bool getDone( void ) {
  return done;
}

// Set song to terminate.
void setDone( void ) {
  done = true;
  resetWavelet();
  updateUI();
}

// Indicates whether the MP3 player is paused.
bool isPaused( void ) {
  return paused;
}

// Set state of pause.
void setPaused( bool v ) {
  paused = v;
}

// Get current song number playing
uint8_t getCurrentSong(void) {
  return song;
}

// Indicates state of shuffle mode.
bool isShuffle( void ) {
  return shuffle;
}

// Set state of shuffle mode.
void setShuffle( bool v ) {
  shuffle = v;
  if (shuffle == true) {
      initialize = true;
  }
  else {
      // deallocates memory
      free(shuffledSongs);
  }
}

// set state to play previous song.
void setSkipBack( void ) {
    skipBack = true;
}

// Set state of userChose.
void setUserChose( bool v ) {
  userChose = v;
}

void initializeSeparateList( void ) {
    uint8_t n = (sizeof(separateList)/sizeof(separateList[0]));
    for (uint8_t i=0; i<n; i++) {
        // initialize play list space, let's assume that 255 is not a valid song
        separateList[i] = 255;
    }
}

void storeSeparateList( void ) {
    // will only initialize separate play list once
    if (initList == true) {
        initializeSeparateList();
        initList = false;
    }
    uint8_t n = (sizeof(separateList)/sizeof(separateList[0]));
    for (uint8_t i=0; i<n; i++) {
        // checks if there is space available in separate play list; 255 is considered empty
        if (separateList[i] == 255) {
            // stores current song into separate play list
            separateList[i] = song;
            break;
        }
    }
}

// Deletes the song from separate play list
void deleteSeparateList( int x ) {
   // Search song 'x' in array
   uint8_t n = (sizeof(separateList)/sizeof(separateList[0]));
   uint8_t i;
   for (i=0; i<n; i++) {
      if (separateList[i] == x)
         break;
   }

   // If x is found in array
   if (i < n) {
     // reduce size of array and shift all elements back starting at original index of x
     n = n - 1;
     for (uint8_t j =i; j<n; j++)
        separateList[j] = separateList[j+1];
   }
}

// Returns the current song that is stored in the separate play list
uint8_t readSeparateList( void ) {
    // will only initialize separate play list once
    if (initList == true) {
        initializeSeparateList();
        initList = false;
    }
    uint8_t v, i;
    uint8_t n = (sizeof(separateList)/sizeof(separateList[0]));
       for (i=0; i<n; i++) {
          if (separateList[i] == song){
              // finds and returns the current song found in play list
             v = song;
             break;
          }
       }
       if (i >= n) {
           // the current song does not currently exist in separate play list
          v = 255;
       }
    return v;
}

bool isListMode( void ) {
    return listMode;
}

void setListMode( bool mode ) {
    nextFavSong = 0;
    listMode = mode;
}
