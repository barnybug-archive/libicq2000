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

#include "PipeExec.h"

PipeExec::PipeExec()
  : fStdIn(NULL), fStdOut(NULL)
{ }

PipeExec::~PipeExec()
{ 
  Close();
}

bool PipeExec::Open(const char *shellcmd)
{
  int pdes_out[2], pdes_in[2];

  if (pipe(pdes_out) < 0) return false;
  if (pipe(pdes_in) < 0) return false;

  switch (pid = fork())
  {
    case -1:                        /* Error. */
    {
      close(pdes_out[0]);
      close(pdes_out[1]);
      close(pdes_in[0]);
      close(pdes_in[1]);
      return false;
      /* NOTREACHED */
    }
  case 0:                         /* Child. */
    {
      if (pdes_out[1] != STDOUT_FILENO) {
        dup2(pdes_out[1], STDOUT_FILENO);
        close(pdes_out[1]);
      }
      
      close(pdes_out[0]);
      
      if (pdes_in[0] != STDIN_FILENO) {
        dup2(pdes_in[0], STDIN_FILENO);
	close(pdes_in[0]);
      }
      close(pdes_in[1]);
      system(shellcmd);
      exit(0);
      /* NOTREACHED */
    }
  }

  /* Parent; assume fdopen can't fail. */
  fStdOut = fdopen(pdes_out[0], "r");
  close(pdes_out[1]);
  fStdIn = fdopen(pdes_in[1], "w");
  close(pdes_in[0]);

  // Set both streams to line buffered
#ifdef SETVBUF_REVERSED
  setvbuf(fStdOut, _IOLBF, (char*)NULL, 0);
  setvbuf(fStdIn, _IOLBF, (char*)NULL, 0);
#else
  setvbuf(fStdOut, (char*)NULL, _IOLBF, 0);
  setvbuf(fStdIn, (char*)NULL, _IOLBF, 0);
#endif

  return true;
}

void PipeExec::Read(char *buf, int size)
{
  int pos = 0;
  int c;
  while (((c = fgetc(fStdOut)) != EOF) && (pos < size)) buf[pos++] = (unsigned char)c;
  buf[pos] = '\0';
}

void PipeExec::Write(const char *buf)
{
  fprintf(fStdIn, "%s", buf);
}

void PipeExec::CloseInput()
{
  fclose(fStdIn);
  fStdIn = NULL;
}

void PipeExec::Close()
{
   int r, pstat;
   struct timeval tv = { 0, 200000 };

   // Close the file descriptors
   if (fStdOut != NULL) fclose(fStdOut);
   if (fStdIn != NULL) fclose(fStdIn);
   fStdOut = fStdIn = NULL;

   if (pid == 0) return;

   // See if the child is still there
   r = waitpid(pid, &pstat, WNOHANG);

   // Return if child has exited or there was an inor
   if (r == pid || r == -1) return;
     
   // Give the process another .2 seconds to die
   select(0, NULL, NULL, NULL, &tv);
   
   // Still there?
   r = waitpid(pid, &pstat, WNOHANG);
   if (r == pid || r == -1) return;
     
   // Try and kill the process
   if (kill(pid, SIGTERM) == -1) return;
   
   // Give it 1 more second to die
   tv.tv_sec = 1;
   tv.tv_usec = 0;
   select(0, NULL, NULL, NULL, &tv);
   
   // See if the child is still there
   r = waitpid(pid, &pstat, WNOHANG);
   if (r == pid || r == -1) return;

   // Kill, kill, keeeiiiiilllllll!!
   kill(pid, SIGKILL);
   // Now he will die for sure
   waitpid(pid, &pstat, 0);

}
