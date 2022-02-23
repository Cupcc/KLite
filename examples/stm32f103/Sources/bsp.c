#include "bsp.h"
#include "stm32f10x.h"

void bsp_init(void)
{
	RCC->APB2ENR |= 1 << 4;
	GPIOC->CRH &= 0xFFFFFF00;
	GPIOC->CRH |= 0x00000033;
}

void bsp_led_on(int num)
{
	GPIOC->ODR |= 1 << (num + 8);
}

void bsp_led_off(int num)
{
	GPIOC->ODR &= ~(1 << (num + 8));
}
