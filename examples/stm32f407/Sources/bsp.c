#include "bsp.h"
#include "stm32f4xx.h"

void bsp_init(void)
{
	RCC->AHB1ENR |= 1 << 2;
	GPIOC->MODER = 0x00000005;
	GPIOC->OTYPER = 0x00000000;
}

void bsp_led_on(int num)
{
	GPIOC->ODR |= 1 << (num);
}

void bsp_led_off(int num)
{
	GPIOC->ODR &= ~(1 << (num));
}
