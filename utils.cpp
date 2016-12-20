#include "updf.h"
#include "utils.h"

void *xcalloc(size_t nmemb, size_t size) {

  void *tmp = calloc(nmemb, size);
  if (!tmp) die("Out of memory\n");

  return tmp;
}

void *xmalloc(size_t size) {

  void *tmp = malloc(size);
  if (!tmp) die("Out of memory\n");

  return tmp;
}

void die(const char fmt[], ...) {

  va_list ap;

  va_start(ap, fmt);

  vfprintf(stderr, fmt, ap);

  va_end(ap);
  exit(1);
}

void warn(const char fmt[], ...) {

  va_list ap;

  va_start(ap, fmt);

  vfprintf(stderr, fmt, ap);

  va_end(ap);
}

u32 usecs(const timeval old, const timeval now) {

  u32 us = (now.tv_sec - old.tv_sec) * 1000 * 1000;
  us += now.tv_usec - old.tv_usec;

  return us;
}

u64 msec() {
  struct timeval t;
  gettimeofday(&t, NULL);

  u64 ms = t.tv_sec * 1000;
  ms += t.tv_usec / 1000;
  return ms;
}

#if DEBUGGING

  void debug_it(char const * fmt, ...)
  {
    va_list ap;

    va_start(ap, fmt);

    vfprintf(stderr, fmt, ap);

    va_end(ap);
  }

  void debug_it(Fl_Box * ctrl, const float value, const char * hint)
  {
    char tmp[20];

    snprintf(tmp, 20, "%5.3f", value);
    ctrl->copy_label(tmp);
    ctrl->tooltip(hint);
    ctrl->redraw_label();
  }

  void debug_it(Fl_Box * ctrl, const u32 value, const char * hint)
  {
    char tmp[20];

    snprintf(tmp, 20, "%u", value);
    ctrl->copy_label(tmp);
    ctrl->tooltip(hint);
    ctrl->redraw_label();
    Fl::check();
  }

  void debug_it(Fl_Box * ctrl, const s32 value, const char * hint)
  {
    char tmp[20];

    snprintf(tmp, 20, "%d", value);
    ctrl->copy_label(tmp);
    ctrl->tooltip(hint);
    ctrl->redraw_label();
    Fl::check();
  }
#endif

