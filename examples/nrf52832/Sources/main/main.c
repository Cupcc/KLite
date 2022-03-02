#include "kernel.h"
#include "daemon.h"
#include "app.h"

static void init(void *arg)
{
	app_init();
	daemon_init();
}

static void idle(void *arg)
{
	kernel_idle();
}

int main(void)
{
	static uint8_t heap[16*1024];
	kernel_init(heap, sizeof(heap));
	thread_create(idle, NULL, 1024);
	thread_create(init, NULL, 1024);
	kernel_start();
}
