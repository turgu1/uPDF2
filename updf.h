#ifndef UPDF_H
#define UPDF_H

#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/time.h>
#include <lzo/lzo1x.h>

#include <QObject>
#include <PDFDoc.h>

#define UPDF_VERSION "2.0.0"

#define DEBUGGING 1

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t  s8;

extern u32 details;

#include "utils.h"

#endif // UPDF_H
