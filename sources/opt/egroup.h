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
#ifndef __EGROUP_H
#define __EGROUP_H

#define EGROUP_OPS_WAIT_ANY    0x00
#define EGROUP_OPS_WAIT_ALL    0x01
#define EGROUP_OPS_AUTO_CLEAR  0x02

typedef struct egroup *egroup_t;

egroup_t egroup_create(void);
void     egroup_delete(egroup_t event);
void     egroup_set(egroup_t event, uint32_t bits);
void     egroup_clear(egroup_t event, uint32_t bits);
uint32_t egroup_wait(egroup_t event, uint32_t bits, uint32_t ops);
uint32_t egroup_timed_wait(egroup_t event, uint32_t bits, uint32_t ops, uint32_t timeout);

#endif
