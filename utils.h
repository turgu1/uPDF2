#ifndef UTILS_H
#define UTILS_H

#include <QString>

#if DEBUGGING
  #define debug(str) dbg(str)
#else
  #define debug(str)
#endif

extern void *xcalloc(size_t nmemb, size_t size);
extern void *xmalloc(size_t size);
extern void      die(const QString & str);
extern void     warn(const QString & str);
extern void     info(const QString & str);
extern void      dbg(const QString & str);
extern u32     usecs(const timeval old, const timeval now);

#endif // UTILS_H
