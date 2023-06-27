#ifndef __LED_H__
#define __LED_H__
#include "driver/gpio.h"
#include "esp_log.h"

#define LED_NET     (1)
#define LED_WORK    (2)


void LED_Init(void);

void netLedOn(void);

void netLedOff(void);

void workLedOn(void);

void workLedOff(void);


#endif /* __CAMERA_H__ */




