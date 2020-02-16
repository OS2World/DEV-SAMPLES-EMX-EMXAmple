/* redir.c (emx+gcc) -- Copyright (c) 1994 by Eberhard Mattes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define FALSE 0
#define TRUE  1

static int my_stderr = 2;


static void write_str (const char *s)
{
  write (my_stderr, s, strlen (s));
}


static void failure (const char *msg)
{
  const char *error;

  error = strerror (errno);
  write_str (msg);
  write_str (": ");
  write_str (error);
  write_str ("\n");
  exit (2);
}


static int xdup (int fd)
{
  int t;

  t = dup (fd);
  if (t < 0)
    failure ("dup()");
  return (t);
}


static int xdup2 (int fd0, int fd1)
{
  int t;

  if (fd1 == my_stderr)
    my_stderr = xdup (my_stderr);
  t = dup2 (fd0, fd1);
  if (t < 0)
    failure ("dup2()");
  return (t);
}


static void usage (void)
{
  write_str ("Usage: redir \"<redirections>\" \"<program>\"\n\n"
             "Redirections (seperate them with spaces):\n"
             "  <file      Redirect standard input from FILE\n"
             "  >file      Redirect standard output to FILE\n"
             "  >>file     Redirect standard output to FILE, appending\n"
             "  n<file     Redirect file descriptor N from FILE\n"
             "  n>file     Redirect file descriptor N to FILE\n"
             "  n<>file    Redirect file descriptor N from and to FILE\n"
             "  <&m        Redirect standard input from file descriptor M\n"
             "  >&m        Redirect standard output to file descriptor M\n"
             "  n<&m       Redirect file descriptor N from file descriptor M\n"
             "  n>&m       Redirect file descriptor N to file descriptor M\n"
             "  <&-        Close standard input\n\n"
             "Example:\n\n"
             "  redir \">output 2>&1\" \"make\"\n");
  exit (1);
}


static void redirections (const char *s)
{
  int fd_explicit, fd_default, fd_source, fd_target, mode;
  char fname[512];
  char f_amp;
  int i;

  for (;;)
    {
      while (isspace ((unsigned char)*s))
        ++s;
      if (*s == 0)
        break;
      fd_explicit = -1;
      if (isdigit ((unsigned char)*s))
        fd_explicit = *s++ - '0';
      switch (*s)
        {
        case '<':
          ++s;
          if (*s == '>' && fd_explicit != -1)
            {
              ++s;
              mode = O_RDWR; f_amp = FALSE;
            }
          else
            {
              mode = O_RDONLY; f_amp = TRUE;
            }
          fd_default = 0;
          break;

        case '>':
          ++s;
          if (*s == '>')
            {
              ++s;
              mode = O_WRONLY|O_CREAT|O_APPEND; f_amp = FALSE;
            }
          else
            {
              mode = O_WRONLY|O_CREAT|O_TRUNC; f_amp = TRUE;
            }
          fd_default = 1;
          break;

        default:
          usage ();
        }
      if (fd_explicit != -1)
        fd_target = fd_explicit;
      else
        fd_target = fd_default;
      fd_source = -1;
      if (*s == '&' && f_amp)
        {
          ++s;
          if (*s == '-' && fd_explicit == -1)
            ++s;                /* fd_source = -1 */
          else if (!isdigit ((unsigned char)*s))
            usage ();
          else
            fd_source = *s++ - '0';
        }
      else
        {
          i = 0;
          while (*s != 0 && !isspace ((unsigned char)*s) && i < sizeof (fname))
            fname[i++] = *s++;
          if (i >= sizeof (fname))
            usage ();
          fname[i] = 0;
          fd_source = open (fname, mode, S_IREAD | S_IWRITE);
          if (fd_source < 0)
            failure (fname);
        }

      if (*s != 0 && !isspace ((unsigned char)*s))
        usage ();

      if (fd_source == -1)
        {
          /* Perhaps this should be postponed until the end of the
             redirections is reached, as relocating my_stderr might
             fill-in that slot, see xdup2(). */

          close (fd_target);
        }
      else if (fd_source == fd_target)
        usage ();
      else
        xdup2 (fd_source, fd_target);
    }
}


static void run_program (const char *s)
{
  int r;

  r = system (s);
  if (r < 0)
    failure (s);
  else
    exit (r);
}


int main (int argc, char *argv[])
{
  if (argc != 3)
    usage ();
  redirections (argv[1]);
  run_program (argv[2]);
  return (0);
}
