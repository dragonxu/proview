/*
 * ProviewR   Open Source Process Control.
 * Copyright (C) 2005-2018 SSAB EMEA AB.
 *
 * This file is part of ProviewR.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ProviewR. If not, see <http://www.gnu.org/licenses/>
 *
 * Linking ProviewR statically or dynamically with other modules is
 * making a combined work based on ProviewR. Thus, the terms and
 * conditions of the GNU General Public License cover the whole
 * combination.
 *
 * In addition, as a special exception, the copyright holders of
 * ProviewR give you permission to, from the build function in the
 * ProviewR Configurator, combine ProviewR with modules generated by the
 * ProviewR PLC Editor to a PLC program, regardless of the license
 * terms of these modules. You may copy and distribute the resulting
 * combined work under the terms of your choice, provided that every
 * copy of the combined work is accompanied by a complete copy of
 * the source code of ProviewR (the version used to produce the
 * combined work), being distributed under the terms of the GNU
 * General Public License plus this exception.
 */

#include "co_debug.h"

#include <stdarg.h>
#include <string.h>
#include <time.h>

int DEBUG = 0;

void print_time(FILE* stream, int fulldate)
{
  time_t t;
  struct tm* tm;
  char Date[11], Time[11];
  time(&t);
  tm = localtime(&t);
  if (fulldate) {
    strftime(Date, 11, "%Y-%m-%d", tm);
    fprintf(stream, "%s ", Date);
  }
  strftime(Time, 11, "%H:%M:%S", tm);
  fprintf(stream, "%s", Time);
}

void dbg_print(const char* file, int line, const char* fmt, ...)
{
  if (DEBUG) {
    // 1. print timestamp
    print_time(stderr);
    // 2. print filename only, without path
    const char* file2 = strrchr(file, '/');
    file2 = file2 ? (file2 + 1) : file;
    fprintf(stderr, " %s:%d: ", file2, line);
    // 3. print the actual debug message
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
  }
}