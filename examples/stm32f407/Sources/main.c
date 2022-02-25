#include "kernel.h"
#include "bsp.h"
#include "app.h"

static void init(void *arg)
{
	app_init();
}

static void idle(void *arg)
{
	kernel_idle();
}

int main(void)
{
	bsp_init();
	kernel_init((void *)0x10000000, 0x10000); //CCM RAM
	thread_create(idle, NULL, 0);
	thread_create(init, NULL, 0);
	kernel_start();
}
