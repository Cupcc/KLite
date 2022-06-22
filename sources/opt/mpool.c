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
	uint32_t *addr_list;
	uint32_t  item_count;
	uint32_t  free_count;
	uint32_t  next_out;
	uint32_t  next_in;
};

mpool_t mpool_create(uint32_t item_size, uint32_t item_count)
{
	uint32_t i;
	uint32_t item;
	uint32_t msize;
	mpool_t  mpool;
	msize = sizeof(struct mpool) + (sizeof(uint32_t *) + item_size) * item_count;
	mpool = heap_alloc(msize);
	if(mpool != NULL)
	{
		memset(mpool, 0, msize);
		mpool->mutex = mutex_create();
		if(mpool->mutex != NULL)
		{
			mpool->addr_list = (uint32_t *)(mpool + 1);
			mpool->item_count = item_count;
			item = (uint32_t)(mpool->addr_list + item_count);
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
	if(mpool->free_count > 0)
	{
		item = mpool->addr_list[mpool->next_out];
		mpool->free_count--;
		mpool->next_out++;
		if(mpool->next_out >= mpool->item_count)
		{
			mpool->next_out = 0;
		}
	}
	mutex_unlock(mpool->mutex);
	return (void *)item;
}

void mpool_free(mpool_t mpool, void *mem)
{
	mutex_lock(mpool->mutex);
	mpool->addr_list[mpool->next_in] = (uint32_t)mem;
	mpool->free_count++;
	mpool->next_in++;
	if(mpool->next_in >= mpool->item_count)
	{
		mpool->next_in = 0;
	}
	mutex_unlock(mpool->mutex);
}
