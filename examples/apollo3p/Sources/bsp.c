#include "bsp.h"
#include "apollo3p.h"

void bsp_init(void)
{
	GPIO->PADKEY = 115;
	GPIO->ENA |= (1 << 14) | (1 << 15);
	GPIO->CFGB = 0xFFFFFFFF;
	GPIO->PADKEY = 0;
}

void bsp_led_on(int num)
{
	GPIO->WTA |= (1 << (num + 14));
}

void bsp_led_off(int num)
{
	GPIO->WTA &= ~(1 << (num + 14));
}
