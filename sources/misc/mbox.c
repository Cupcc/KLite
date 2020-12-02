/*
* 描述: 简易的消息邮箱
*       没有实现消息列队，只能保存一个状态值
* 应用场景: 常用于"命令-应答"模式的程序逻辑, 一个线程负责发命令并等待应答，另一个线程负责接收应答
* 作者: 蒋晓岗<kerndev@foxmail.com>
*/
#include "kernel.h"
#include "mbox.h"

bool mbox_init(mbox_t *mbox)
{
	mbox->data = 0;
	mbox->sem = sem_create(0, 1);
	if(mbox->sem != NULL)
	{
		return true;
	}
	return false;
}

void mbox_delete(mbox_t *mbox)
{
	sem_delete(mbox->sem);
}

void mbox_send(mbox_t *mbox, uint32_t data)
{
	mbox->data = data;
	sem_post(mbox->sem);
}

bool mbox_recv(mbox_t *mbox, uint32_t *p_data, uint32_t timeout)
{
	bool ret;
	ret = sem_timed_wait(mbox->sem, timeout);
	*p_data = mbox->data;
	return ret;
}
