/*
 * Pipe Execution helper class - for some fun in the shell example,
 * this doesn't particularly have anything to do with libicq2000 :-)
 *
 * Copyright (C) 2002 Barnaby Gray <barnaby@beedesign.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef EXAMPLE_PIPEEXEC_H
#define EXAMPLE_PIPEEXEC_H

#include <config.h>

#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#ifndef WEXITSTATUS
# define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)
#endif
#ifndef WIFEXITED
# define WIFEXITED(stat_val) (((stat_val) & 255) == 0)
#endif

#include <signal.h>

// ------------------------------------------------------------------
// Pipe execution class
// ------------------------------------------------------------------

class PipeExec {
 private:
  FILE *fStdIn, *fStdOut;
  int pid;

 public:
  PipeExec();
  ~PipeExec();

  bool Open(const char *cmd);
  void Read(char *buf, int size);
  void Write(const char *buf);
  void CloseInput();
  void Close();
};

#endif // EXAMPLE_PIPEEXEC_H
