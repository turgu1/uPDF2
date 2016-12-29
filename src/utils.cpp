/*
Copyright (C) 2015 Lauri Kasanen
Modifications Copyright (C) 2017 Guy Turcotte

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <QObject>
#include <QTextStream>

#include "updf.h"

void *xcalloc(size_t nmemb, size_t size) {

  void *tmp = calloc(nmemb, size);
  if (!tmp) {
    qFatal("Out of memory\n");
  }
  return tmp;
}

void *xmalloc(size_t size) {

  void *tmp = malloc(size);
  if (!tmp) {
    qFatal("Out of memory\n");
  }
  return tmp;
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

