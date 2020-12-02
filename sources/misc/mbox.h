#ifndef __MBOX_H
#define __MBOX_H

typedef struct
{
	sem_t sem;
	uint32_t data;
}mbox_t;

bool  mbox_create(mbox_t *mbox);
void  mbox_delete(mbox_t *mbox);
void  mbox_send(mbox_t *mbox, uint32_t data);
bool  mbox_recv(mbox_t *mbox, uint32_t *p_data, uint32_t timeout);

#endif
