#include "led.h"

void LED_Init(void)
{
    gpio_pad_select_gpio(LED_NET);
    gpio_set_direction(LED_NET, GPIO_MODE_OUTPUT);//  把这个GPIO作为输出
    
    gpio_pad_select_gpio(LED_WORK);
    gpio_set_direction(LED_WORK, GPIO_MODE_OUTPUT);//  把这个GPIO作为输出
}

void netLedOn(void)
{
    gpio_set_level(LED_NET, 0);
}

void netLedOff(void)
{
    gpio_set_level(LED_NET, 1);
}

void workLedOn(void)
{
    gpio_set_level(LED_WORK, 0);
}

void workLedOff(void)
{
    gpio_set_level(LED_WORK, 1);
}








