#include "kernel.h"
#include "sdk.h"
#include "app.h"

static void init(void *p)
{
	sdk_init();
	app_init();
	sdk_loop();
}

static void idle(void *p)
{
	kernel_idle();
}

int main(void)
{
	static uint8_t heap[8192];
	kernel_init((uint32_t)heap, sizeof(heap));
	thread_create(init, 0, 2048);
	thread_create(idle, 0, 1024);
	kernel_start();
}
