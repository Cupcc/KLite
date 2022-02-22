#include "bsp.h"
#include "f1c100s.h"

void bsp_init(void)
{
	GPIOE->CFG[0] = 0x11111111;
}

void bsp_led_on(int num)
{
	GPIOE->DATA |= 1 << (num + 5);
}

void bsp_led_off(int num)
{
	GPIOE->DATA &= ~(1 << (num + 5));
}
