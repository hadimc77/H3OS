/**
 * H3OS — Fundamental types
 * The Future Starts Here.
 */
#ifndef H3OS_TYPES_H
#define H3OS_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef signed char        i8;
typedef signed short       i16;
typedef signed int         i32;
typedef signed long long   i64;

typedef u64 size_t;
typedef i64 ssize_t;
typedef u64 uintptr_t;
typedef i64 intptr_t;
typedef u64 phys_addr_t;
typedef u64 virt_addr_t;

typedef u8  bool;
#define true  1
#define false 0

#define NULL ((void*)0)

#define H3OS_UNUSED(x) ((void)(x))
#define H3OS_ALIGN_UP(x, a)   (((x) + ((a) - 1)) & ~((a) - 1))
#define H3OS_ALIGN_DOWN(x, a) ((x) & ~((a) - 1))
#define H3OS_MIN(a, b) ((a) < (b) ? (a) : (b))
#define H3OS_MAX(a, b) ((a) > (b) ? (a) : (b))
#define H3OS_ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define PAGE_SIZE       4096ULL
#define PAGE_SHIFT      12
#define HUGE_PAGE_SIZE  (2ULL * 1024 * 1024)

#define KERNEL_STACK_SIZE (16 * 1024)

#ifdef __cplusplus
}
#endif

#endif /* H3OS_TYPES_H */
