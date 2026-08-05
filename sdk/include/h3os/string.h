/**
 * H3OS — Kernel string / memory helpers (freestanding)
 */
#ifndef H3OS_STRING_H
#define H3OS_STRING_H

#include <h3os/types.h>

#ifdef __cplusplus
extern "C" {
#endif

void*  memset(void* dst, int c, size_t n);
void*  memcpy(void* dst, const void* src, size_t n);
void*  memmove(void* dst, const void* src, size_t n);
int    memcmp(const void* a, const void* b, size_t n);

size_t strlen(const char* s);
int    strcmp(const char* a, const char* b);
int    strncmp(const char* a, const char* b, size_t n);
char*  strcpy(char* dst, const char* src);
char*  strncpy(char* dst, const char* src, size_t n);
char*  strcat(char* dst, const char* src);

void   itoa(i64 value, char* buf, int base);
void   utoa(u64 value, char* buf, int base);

#ifdef __cplusplus
}
#endif

#endif /* H3OS_STRING_H */
