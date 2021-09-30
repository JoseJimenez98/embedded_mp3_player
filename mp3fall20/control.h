/*
 * control.h
 *
 *  Created on: Aug 13, 2018
 *      Author: khughes
 *
 */

#ifndef _CONTROL_H
#define _CONTROL_H

#include <stdint.h>
#include <stdbool.h>

uint8_t getNumSongs( void );
void setNumSongs( uint8_t num );
uint8_t getSong( void );
void setSong( uint8_t v );
void setDone( void );
bool isDone( void );
void setPaused( bool v );
bool isPaused( void );
void setShuffle( bool v );
bool isShuffle( void );
void setUserChose( bool v );
void initializeShuffle( void );
void setSkipBack( void );
uint8_t getCurrentSong(void);
bool getDone( void );
void initializeSeparateList( void );
void storeSeparateList( void );
void deleteSeparateList( int x );
uint8_t readSeparateList( void );
bool isListMode( void );
void setListMode( bool mode );

#endif // _CONTROL_H
