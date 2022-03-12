/******************************************************************************
* Copyright (c) 2015-2022 jiangxiaogang<kerndev@foxmail.com>
*
* This file is part of KLite distribution.
*
* KLite is free software, you can redistribute it and/or modify it under
* the MIT Licence.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
******************************************************************************/
#include <string.h>
#include "kernel.h"
#include "mpool.h"

struct mpool
{
	mutex_t   mutex;
	uint32_t *item_addr;
	uint32_t  item_count;
	uint32_t  item_free;
	uint32_t  next_alloc;
	uint32_t  next_free;
};

mpool_t mpool_create(uint32_t item_size, uint32_t item_count)
{
	uint32_t i;
	uint32_t item;
	mpool_t mpool;
	mpool = heap_alloc((item_size + item_count * sizeof(void *)) + sizeof(struct mpool));
	if(mpool != NULL)
	{
		memset(mpool, 0, sizeof(sizeof(struct mpool)));
		mpool->mutex = mutex_create();
		if(mpool->mutex != NULL)
		{
			mpool->item_addr = (uint32_t *)(mpool + 1);
			mpool->item_count = item_count;
			item = (uint32_t)(mpool->item_addr + item_count);
			for(i = 0; i < item_count; i++)
			{
				mpool_free(mpool, (void *)item);
				item += item_size;
			}
			return mpool;
		}
		heap_free(mpool);
	}
	return NULL;
}

void mpool_delete(mpool_t mpool)
{
	mutex_delete(mpool->mutex);
	heap_free(mpool);
}

void *mpool_alloc(mpool_t mpool)
{
	uint32_t item = 0;
	mutex_lock(mpool->mutex);
	if(mpool->item_free > 0)
	{
		item = mpool->item_addr[mpool->next_alloc];
		if(++mpool->next_alloc >= mpool->item_count)
		{
			mpool->next_alloc = 0;
		}
		mpool->item_free--;
	}
	mutex_unlock(mpool->mutex);
	return (void *)item;
}

void mpool_free(mpool_t mpool, void *mem)
{
	uint32_t item = (uint32_t)mem;
	mutex_lock(mpool->mutex);
	mpool->item_addr[mpool->next_free] = item;
	if(++mpool->next_free >= mpool->item_count)
	{
		mpool->next_free = 0;
	}
	mpool->item_free++;
	mutex_unlock(mpool->mutex);
}
