#include "bsp.h"
#include "nrf51.h"

void bsp_init(void)
{
	NRF_GPIO->OUT = 0;
	NRF_GPIO->DIR |= (1<<1)|(1<<22)|(1<<23);
	NRF_GPIO->PIN_CNF[1] = 0x03;
	NRF_GPIO->PIN_CNF[22] = 0x03;
	NRF_GPIO->PIN_CNF[23] = 0x03;
}

void bsp_led_on(int num)
{
	NRF_GPIO->OUTCLR |= 1 << (22 + num);
}

void bsp_led_off(int num)
{
	NRF_GPIO->OUTSET |= 1 << (22 + num);
}
