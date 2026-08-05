/**
 * H3OS — Freestanding string and memory utilities
 */
#include <h3os/string.h>

void* memset(void* dst, int c, size_t n) {
    u8* p = (u8*)dst;
    while (n--) *p++ = (u8)c;
    return dst;
}

void* memcpy(void* dst, const void* src, size_t n) {
    u8* d = (u8*)dst;
    const u8* s = (const u8*)src;
    while (n--) *d++ = *s++;
    return dst;
}

void* memmove(void* dst, const void* src, size_t n) {
    u8* d = (u8*)dst;
    const u8* s = (const u8*)src;
    if (d < s) {
        while (n--) *d++ = *s++;
    } else {
        d += n;
        s += n;
        while (n--) *--d = *--s;
    }
    return dst;
}

int memcmp(const void* a, const void* b, size_t n) {
    const u8* x = (const u8*)a;
    const u8* y = (const u8*)b;
    while (n--) {
        if (*x != *y) return (int)*x - (int)*y;
        x++;
        y++;
    }
    return 0;
}

size_t strlen(const char* s) {
    size_t n = 0;
    while (s[n]) n++;
    return n;
}

int strcmp(const char* a, const char* b) {
    while (*a && (*a == *b)) { a++; b++; }
    return (int)(u8)*a - (int)(u8)*b;
}

int strncmp(const char* a, const char* b, size_t n) {
    while (n && *a && (*a == *b)) { a++; b++; n--; }
    if (n == 0) return 0;
    return (int)(u8)*a - (int)(u8)*b;
}

char* strcpy(char* dst, const char* src) {
    char* d = dst;
    while ((*d++ = *src++));
    return dst;
}

char* strncpy(char* dst, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i]; i++) dst[i] = src[i];
    for (; i < n; i++) dst[i] = '\0';
    return dst;
}

char* strcat(char* dst, const char* src) {
    char* d = dst + strlen(dst);
    while ((*d++ = *src++));
    return dst;
}

void utoa(u64 value, char* buf, int base) {
    char tmp[65];
    int i = 0;
    if (base < 2 || base > 16) { buf[0] = '\0'; return; }
    if (value == 0) { buf[0] = '0'; buf[1] = '\0'; return; }
    while (value) {
        int digit = (int)(value % (u64)base);
        tmp[i++] = (char)(digit < 10 ? '0' + digit : 'a' + digit - 10);
        value /= (u64)base;
    }
    while (i--) *buf++ = tmp[i];
    *buf = '\0';
}

void itoa(i64 value, char* buf, int base) {
    if (value < 0 && base == 10) {
        *buf++ = '-';
        utoa((u64)(-value), buf, base);
    } else {
        utoa((u64)value, buf, base);
    }
}
