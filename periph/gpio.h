/*
 * gpio.h
 *
 *  Created on: July 29, 2018
 *      Author: khughes
 *
 * GPIO register definitions.  See Chapter 10 of the TM4C1294 datasheet 
 * for complete information.
 */

#ifndef _GPIO_H
#define _GPIO_H

#include <stdint.h>

// Peripheral base addresses.
#define GPIO_PORTA              ((volatile uint32_t *)0x40058000)   //added port A
#define GPIO_PORTB              ((volatile uint32_t *)0x40059000)   //added port B
#define GPIO_PORTC              ((volatile uint32_t *)0x4005A000)   //added port C
#define GPIO_PORTD              ((volatile uint32_t *)0x4005B000)   //added port D
#define GPIO_PORTE              ((volatile uint32_t *)0x4005C000)   //added port E
#define GPIO_PORTF              ((volatile uint32_t *)0x4005D000)   //added port F
#define GPIO_PORTG              ((volatile uint32_t *)0x4005E000)   //added port G
#define GPIO_PORTH              ((volatile uint32_t *)0x4005F000)   //added port H
#define GPIO_PORTJ              ((volatile uint32_t *)0x40060000)   //added port J
#define GPIO_PORTK              ((volatile uint32_t *)0x40061000)   //added port K
#define GPIO_PORTL              ((volatile uint32_t *)0x40062000)   //added port L
#define GPIO_PORTM              ((volatile uint32_t *)0x40063000)   //added port M
#define GPIO_PORTN              ((volatile uint32_t *)0x40064000)   //added port N
#define GPIO_PORTP              ((volatile uint32_t *)0x40065000)   //added port P
#define GPIO_PORTQ              ((volatile uint32_t *)0x40066000)   //added port Q

// Peripheral register offsets and special fields
enum {
#define   GPIO_PIN_0              (1 << 0) // pin 0
#define   GPIO_PIN_1              (1 << 1) // added pin 1
#define   GPIO_PIN_2              (1 << 2) // added pin 2
#define   GPIO_PIN_3              (1 << 3) // added pin 3
#define   GPIO_PIN_4              (1 << 4) // added pin 4
#define   GPIO_PIN_5              (1 << 5) // added pin 5
#define   GPIO_PIN_6              (1 << 6) // added pin 6
#define   GPIO_PIN_7              (1 << 7) // added pin 7

#define   GPIO_ALLPINS            0b11111111 // pins 0 to 7
  GPIO_DIR  =   (0x400 >> 2),   // added GPIO DIR
  GPIO_AFSEL=   (0x420 >> 2),   // added GPIO AFSEL
  GPIO_ODR  =   (0x50c >> 2),   // added GPIO ODR
  GPIO_PUR  =   (0x510 >> 2),   // added GPIO PUR
  GPIO_DEN  =   (0x51c >> 2),   // added GPIO DEN
  GPIO_LOCK =   (0x520 >> 2),   // added GPIO LOCK
  GPIO_CR   =   (0x524 >> 2),   // added GPIO CR
  GPIO_PCTL =   (0x52c >> 2),   // added GPIO PCTL
};

#endif // _GPIO_H
