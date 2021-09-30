/*
 * keypad.h
 *
 *  Created on:  July 21, 2019
 *  Modified on: Oct 2, 2019 
 *      Author: khughes
 */

#ifndef __KEYPAD_H__
#define __KEYPAD_H__

#include <stdint.h>
#include <stdbool.h>

struct portinfo {
   uint8_t pins[4];
   volatile uint32_t *base;
};

void initKeypad( const struct portinfo *col, const struct portinfo *row );
bool getKey( uint8_t *col, uint8_t *row );

#endif //  __KEYPAD_H__
