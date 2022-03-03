#include "kernel.h"
#include "bsp.h"
#include "app.h"

static void init(void *arg)
{
    bsp_init();
	app_init();
}

static void idle(void *arg)
{
	kernel_idle();
}

int main(void)
{
	static uint8_t heap[64*1024];
	kernel_init(heap, sizeof(heap));
	thread_create(idle, NULL, 0);
	thread_create(init, NULL, 0);
	kernel_start();
}
