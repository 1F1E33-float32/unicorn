/*
 * os-win32.c
 *
 * Copyright (c) 2003-2008 Fabrice Bellard
 * Copyright (c) 2010-2016 Red Hat, Inc.
 *
 * QEMU library functions for win32 which are shared between QEMU and
 * the QEMU tools.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * The implementation of g_poll (functions poll_rest, g_poll) at the end of
 * this file are based on code from GNOME glib-2 and use a different license,
 * see the license comment there.
 */

#include <uc_priv.h>

#include "qemu/osdep.h"
#include <windows.h>
#include "qemu-common.h"
#include "sysemu/sysemu.h"

#if defined(_WIN64) && defined(_MSC_VER)
/* Keep the assembly context restore in sync with the MSVC jump buffer ABI. */
#define CHECK_JUMP_OFFSET(field, offset)                                       \
    _Static_assert(offsetof(_JUMP_BUFFER, field) == (offset),                  \
                   "Unexpected MSVC jump buffer offset: " #field)

_Static_assert(sizeof(jmp_buf) == 256, "Unexpected MSVC jump buffer size");
CHECK_JUMP_OFFSET(Rbx, 0x08);
CHECK_JUMP_OFFSET(Rsp, 0x10);
CHECK_JUMP_OFFSET(Rbp, 0x18);
CHECK_JUMP_OFFSET(Rsi, 0x20);
CHECK_JUMP_OFFSET(Rdi, 0x28);
CHECK_JUMP_OFFSET(R12, 0x30);
CHECK_JUMP_OFFSET(R13, 0x38);
CHECK_JUMP_OFFSET(R14, 0x40);
CHECK_JUMP_OFFSET(R15, 0x48);
CHECK_JUMP_OFFSET(Rip, 0x50);
CHECK_JUMP_OFFSET(MxCsr, 0x58);
CHECK_JUMP_OFFSET(FpCsr, 0x5C);
CHECK_JUMP_OFFSET(Xmm6, 0x60);
CHECK_JUMP_OFFSET(Xmm7, 0x70);
CHECK_JUMP_OFFSET(Xmm8, 0x80);
CHECK_JUMP_OFFSET(Xmm9, 0x90);
CHECK_JUMP_OFFSET(Xmm10, 0xA0);
CHECK_JUMP_OFFSET(Xmm11, 0xB0);
CHECK_JUMP_OFFSET(Xmm12, 0xC0);
CHECK_JUMP_OFFSET(Xmm13, 0xD0);
CHECK_JUMP_OFFSET(Xmm14, 0xE0);
CHECK_JUMP_OFFSET(Xmm15, 0xF0);

#undef CHECK_JUMP_OFFSET
#endif

void *qemu_oom_check(void *ptr)
{
    if (ptr == NULL) {
        fprintf(stderr, "Failed to allocate memory: %lu\n", GetLastError());
        abort();
    }
    return ptr;
}

void *qemu_try_memalign(size_t alignment, size_t size)
{
    void *ptr;

    if (!size) {
        abort();
    }
    ptr = VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);
    //trace_qemu_memalign(alignment, size, ptr);
    return ptr;
}

void *qemu_memalign(size_t alignment, size_t size)
{
    return qemu_oom_check(qemu_try_memalign(alignment, size));
}

static int get_allocation_granularity(void)
{
    SYSTEM_INFO system_info;

    GetSystemInfo(&system_info);
    return system_info.dwAllocationGranularity;
}

void *qemu_anon_ram_alloc(struct uc_struct *uc, size_t size, uint64_t *align)
{
    void *ptr;

    ptr = VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);
    // trace_qemu_anon_ram_alloc(size, ptr);

    if (ptr && align) {
        *align = MAX(get_allocation_granularity(), getpagesize());
    }
    return ptr;
}

void qemu_vfree(void *ptr)
{
    //trace_qemu_vfree(ptr);
    if (ptr) {
        VirtualFree(ptr, 0, MEM_RELEASE);
    }
}

void qemu_anon_ram_free(struct uc_struct *uc, void *ptr, size_t size)
{
    //trace_qemu_anon_ram_free(ptr, size);
    if (ptr) {
        VirtualFree(ptr, 0, MEM_RELEASE);
    }
}

int getpagesize(void)
{
    SYSTEM_INFO system_info;

    GetSystemInfo(&system_info);
    return system_info.dwPageSize;
}
