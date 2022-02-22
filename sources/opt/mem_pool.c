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
#include "mem_pool.h"

struct mem_pool
{
	mutex_t   mutex;
	uint32_t *item_addr;
	uint32_t  item_count;
	uint32_t  item_free;
	uint32_t  next_alloc;
	uint32_t  next_free;
};

mem_pool_t mem_pool_create(uint32_t item_size, uint32_t item_count)
{
	uint32_t i;
	uint32_t item;
	mem_pool_t mem_pool;
	mem_pool = heap_alloc((item_size + item_count * sizeof(void *)) + sizeof(struct mem_pool));
	if(mem_pool != NULL)
	{
		memset(mem_pool, 0, sizeof(sizeof(struct mem_pool)));
		mem_pool->mutex = mutex_create();
		if(mem_pool->mutex != NULL)
		{
			mem_pool->item_addr = (uint32_t *)(mem_pool + 1);
			mem_pool->item_count = item_count;
			item = (uint32_t)(mem_pool->item_addr + item_count);
			for(i = 0; i < item_count; i++)
			{
				mem_pool_free(mem_pool, (void *)item);
				item += item_size;
			}
			return mem_pool;
		}
		heap_free(mem_pool);
	}
	return NULL;
}

void mem_pool_delete(mem_pool_t mem_pool)
{
	mutex_delete(mem_pool->mutex);
	heap_free(mem_pool);
}

void *mem_pool_alloc(mem_pool_t mem_pool)
{
	uint32_t item = 0;
	mutex_lock(mem_pool->mutex);
	if(mem_pool->item_free > 0)
	{
		item = mem_pool->item_addr[mem_pool->next_alloc];
		if(++mem_pool->next_alloc >= mem_pool->item_count)
		{
			mem_pool->next_alloc = 0;
		}
		mem_pool->item_free--;
	}
	mutex_unlock(mem_pool->mutex);
	return (void *)item;
}

void mem_pool_free(mem_pool_t mem_pool, void *mem)
{
	uint32_t item = (uint32_t)mem;
	mutex_lock(mem_pool->mutex);
	mem_pool->item_addr[mem_pool->next_free] = item;
	if(++mem_pool->next_free >= mem_pool->item_count)
	{
		mem_pool->next_free = 0;
	}
	mem_pool->item_free++;
	mutex_unlock(mem_pool->mutex);
}
