/* rmdirs.c (emx+gcc) -- Copyright (c) 1994 by Eberhard Mattes */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <getopt.h>

typedef unsigned char uchar;

static uchar recursive;
static uchar parents;
static uchar verbose;
static uchar delim;

#define DIR_DELIM_P(c) ((c) == '/' || (c) == '\\')

/* Return a non-zero value if NAME refers to a root directory. */

static int is_root (const uchar *name)
{
  if (isalpha (name[0]) && name[1] == ':')
    name += 2;
  else if (DIR_DELIM_P (name[0]) && DIR_DELIM_P (name[1]))
    {
      /* UNC */

      name += 2;
      while (*name != 0 && !DIR_DELIM_P (*name))
        ++name;
    }
  if (DIR_DELIM_P (*name))
    ++name;
  return (*name == 0);
}


/* Return a positive value if directory NAME is empty.  Return zero if
   directory NAME is not empty.  Return a negative value on error. */

static int is_empty (const uchar *name)
{
  DIR *d;
  struct dirent *e;

  /* Cannot use st_nlink. */

  d = opendir (name);
  if (d == NULL)
    {
      perror (name);
      return (-1);
    }
  while ((e = readdir (d)) != NULL
         && strcmp (e->d_name, ".") != 0 && strcmp (e->d_name, "..") != 0)
    ;
  closedir (d);
  return (e == NULL);
}


/* Return 0 on success. */

static int remove_dir (const uchar *name)
{
  if (verbose > 0)
    printf ("Removing %s\n", name);
  if (rmdir (name) == 0)
    return (0);
  perror (name);
  return (-1);
}


/* Return a positive value if directory NAME is empty (after removing
   subdirectories).  Return zero if directory NAME is not empty.
   Return a negative value on error. */

static int recurse (uchar *name)
{
  DIR *d;
  struct dirent *e;
  struct stat s;
  int rc, t;
  size_t len;

  d = opendir (name);
  if (d == NULL)
    {
      perror (name);
      return (-1);
    }
  rc = 0;
  len = strlen (name);
  while ((e = readdir (d)) != NULL)
    if (strcmp (e->d_name, ".") != 0 && strcmp (e->d_name, "..") != 0)
      {
        /* TODO: check length */
        name[len] = delim;
        strcpy (name + len + 1, e->d_name);
        if (stat (name, &s) != 0)
          {
            perror (name); rc = -1;
            break;
          }
        if (S_ISDIR (s.st_mode))
          {
            t = recurse (name);
            if (t < 0)
              {
                rc = -1; break;
              }
            if (t != 0)
              {
                ++rc;
                if (verbose > 1)
                  printf ("Not removing %s\n", name);
              }
            else if (remove_dir (name) != 0)
              {
                rc = -1; break;
              }
          }
        else
          ++rc;
      }
  closedir (d);
  name[len] = 0;
  return (rc);
}


/* Return 0 on success. */

static int process (const uchar *name)
{
  static uchar tmp[FILENAME_MAX];
  size_t len;
  int t;

  len = strlen (name);
  if (len >= sizeof (tmp))
    {
      fprintf (stderr, "%s: filename too long\n", name);
      return (-1);
    }
  delim = (strchr (name, '\\') != NULL ? '\\' : '/');
  memcpy (tmp, name, len);
  while (len > 0 && DIR_DELIM_P (tmp[len-1]))
    --len;
  tmp[len] = 0;
  if (recursive)
    {
      if (recurse (tmp) < 0)
        return (-1);
    }
  if (remove_dir (tmp) != 0)
    return (-1);
  if (parents)
    for (;;)
      {
        while (len > 0 && !DIR_DELIM_P (tmp[len-1]))
          --len;
        if (len > 0) --len;
        tmp[len] = 0;
        if (is_root (tmp)) break;
        t =  is_empty (tmp);
        if (t < 0) return (-1);
        if (!t)
          {
            if (verbose > 1)
              printf ("Not removing %s\n", tmp);
            break;
          }
        if (remove_dir (tmp) != 0)
          return (-1);
      }
  return (0);
}


int main (int argc, char *argv[])
{
  int c, i, result;

  _wildcard (&argc, &argv);
  _response (&argc, &argv);

  while ((c = getopt (argc, argv, "prv")) != -1)
    switch (c)
      {
      case 'r':
        recursive = 1;
        break;
        
      case 'p':
        parents = 1;
        break;
        
      case 'v':
        ++verbose;
        break;
        
      default:
        fprintf (stderr, "rmdirs: invalid option -%c\n", optopt);
        exit (1);
      }

  if (optind >= argc)
    {
      puts ("Usage: rmdirs [-prv] <directory>...\n\n"
            "Options:\n"
            " -p  remove explicit parent directories if being emptied\n"
            " -r  recursively remove empty directories\n"
            " -v  be verbose\n"
            " -vv be very verbose");
      exit (1);
    }

  result = 0;
  for (i = optind; i < argc; ++i)
    if (process (argv[i]) != 0)
      result = 1;

  return (result);
}
