#ifndef __GAMEPADREPORT_H_
#define __GAMEPADREPORT_H_

#include <stdint.h>

struct gamepad_report_t
{
  int8_t x, y;
  uint8_t hatSwitch;
  uint8_t buttons;
};

#endif
