#include "bsp.h"
#include "stm32f10x.h"

void bsp_init(void)
{
	RCC->APB2ENR |= 1 << 2;
	GPIOA->CRH &= 0xFFFFFF00;
	GPIOA->CRH |= 0x0000033;
}

void bsp_led_on(int num)
{
	GPIOA->BSRR |= 1 << (num);
}

void bsp_led_off(int num)
{
	GPIOA->BRR &= ~(1 << (num));
}
