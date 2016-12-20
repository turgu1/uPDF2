#ifndef UTILS_H
#define UTILS_H

#include "updf.h"

extern void *xcalloc(size_t nmemb, size_t size);
extern void *xmalloc(size_t size);
extern void die(const char fmt[], ...);
extern void warn(const char fmt[], ...);
extern u32 usecs(const timeval old, const timeval now);

#endif // UTILS_H
