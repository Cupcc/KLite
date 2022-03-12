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
#include "fifo.h"
#include "bstream.h"

struct bstream
{
	fifo_t  fifo;
	mutex_t mutex;
	event_t empty;
	event_t full;
};

bstream_t bstream_create(uint32_t size)
{
	struct bstream *stream;
	stream = heap_alloc(sizeof(struct bstream) + size);
	if(stream != NULL)
	{
		fifo_init(&stream->fifo, stream + 1, size);
		stream->mutex = mutex_create();
		stream->empty = event_create(true);
		stream->full  = event_create(true);
		if(stream->full == NULL)
		{
			return NULL;
		}
	}
	return stream;
}

void bstream_delete(bstream_t stream)
{
	mutex_delete(stream->mutex);
	event_delete(stream->empty);
	event_delete(stream->full);
	heap_free(stream);
}

void bstream_clear(bstream_t stream)
{
	mutex_lock(stream->mutex);
	fifo_clear(&stream->fifo);
	mutex_unlock(stream->mutex);
	event_set(stream->empty);
}

uint32_t bstream_write(bstream_t stream, void *buf, uint32_t len, uint32_t timeout)
{
	uint8_t *ptr = (uint8_t *)buf;
	uint32_t sum = 0;
	uint32_t ttl;
	while(1)
	{
		mutex_lock(stream->mutex);
		ttl = fifo_write(&stream->fifo, buf, len);
		mutex_unlock(stream->mutex);
		event_set(stream->full);
		ptr += ttl;
		sum += ttl;
		len -= ttl;
		if((timeout > 0) && (len > 0))
		{
			timeout = event_timed_wait(stream->empty, timeout);
			continue;
		}
		return sum;
	}
}

uint32_t bstream_read(bstream_t stream, void *buf, uint32_t len, uint32_t timeout)
{
	uint8_t *ptr = (uint8_t *)buf;
	uint32_t sum = 0;
	uint32_t ttl;
	while(1)
	{
		mutex_lock(stream->mutex);
		ttl = fifo_read(&stream->fifo, buf, len);
		mutex_unlock(stream->mutex);
		event_set(stream->empty);
		ptr += ttl;
		sum += ttl;
		len -= ttl;
		if((timeout > 0) && (len > 0))
		{
			timeout = event_timed_wait(stream->full, timeout);
			continue;
		}
		return sum;
	}
}
