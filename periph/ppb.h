/*
 * ppb.h
 *
 *  Created on: Sep 9, 2020
 *      Author: Jose Jimenez
 */

#ifndef PPB_H_
#define PPB_H_

#include <stdint.h>

#define PPB                     ((volatile uint32_t *) 0xe000e000)

enum {
  PPB_EN0 = (0x100 >> 2),
#define   PPB_EN0_TIMER2A    (1 << 23)
  PPB_STCTRL = (0x10 >> 2),
#define   PPB_STCTRL_COUNT   (1<<16)
#define   PPB_STCTRL_CLK_SRC (1<<2)
#define   PPB_STCTRL_INTEN   (1<<1)
#define   PPB_STCTRL_ENABLE  (1<<0)
  PPB_STRELOAD,
  PPB_STCURRENT,
};


#endif /* PPB_H_ */
