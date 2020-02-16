/* which.c (emx+gcc) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <errno.h>
#include <sys/param.h>
#define INCL_DOSMODULEMGR
#define INCL_DOSERRORS
#include <os2.h>

#define FALSE 0
#define TRUE  1

#define IS_DELIM(c) ((c) == '\\' || (c) == '/' || (c) == ':')

static const char * const ext_list[] =
{
  ".com", ".exe", ".cmd", NULL
};


static int try (const char *dir, const char *name)
{
  char path[MAXPATHLEN];
  const char *p, *q;
  int len, ok, i;

  ok = FALSE;
  if (strlen (dir) >= sizeof (path))
    return (ok);
  strcpy (path, dir);
  len = strlen (path);
  if (len > 0 && !IS_DELIM (path[len-1]))
    {
      if (len + 1 >= sizeof (path))
        return (ok);
      path[len++] = (strchr (path, '/') != NULL ? '/' : '\\');
      path[len] = 0;
    }
  if (strlen (path) + strlen (name) >= sizeof (path))
    return (ok);
  strcat (path, name);
  q = path;
  for (p = path; *p != 0; ++p)
    if (IS_DELIM (*p))
      q = p + 1;
  if (strchr (q, '.') != NULL)
    {
      if (access (path, 4) == 0)
        {
          printf ("%s\n", path);
          ok = TRUE;
        }
    }
  len = strlen (path);
  if (!((len > 0 && path[len-1] == '.') || len + 4 >= sizeof (path)))
    {
      for (i = 0; ext_list[i] != NULL; ++i)
        {
          strcpy (path+len, ext_list[i]);
          if (access (path, 4) == 0)
            {
              printf ("%s\n", path);
              ok = TRUE;
            }
        }
    }
  return (ok);
}


static int find_prog (const char *name, const char *path)
{
  int ok, j;
  char dir[MAXPATHLEN];
  const char *list, *end;

  ok = try ("", name);
  if (path != NULL && strpbrk (name, "/\\:") == NULL)
    {
      list = path;
      for (;;)
        {
          while (*list == ' ' || *list == '\t') ++list;
          if (*list == 0) break;
          end = list;
          while (*end != 0 && *end != ';') ++end;
          j = end - list;
          while (j > 0 && (list[j-1] == ' ' || list[j-1] == '\t')) --j;
          if (j != 0 && j < sizeof (dir))
            {
              memcpy (dir, list, j);
              dir[j] = 0;
              ok |= try (dir, name);
            }
          if (*end == 0) break;
          list = end + 1;
        }
    }
  if (!ok)
    fprintf (stderr, "%s not found\n", name);
  return (ok);
}


static int find_dll (const char *name)
{
  CHAR buf[MAXPATHLEN];
  HMODULE hmod;
  APIRET rc;

  rc = DosLoadModule (buf, sizeof (buf), name, &hmod);
  if (rc == ERROR_FILE_NOT_FOUND)
    {
      fprintf (stderr, "%s not found\n", name);
      return (FALSE);
    }
  if (rc != 0)
    {
      fprintf (stderr, "%s not found (rc=%lu)\n", name, rc);
      return (FALSE);
    }
  rc = DosQueryModuleName (hmod, sizeof (buf), buf);
  DosFreeModule (hmod);
  if (rc != 0)
    {
      fprintf (stderr, "%s not found (rc=%lu)\n", name, rc);
      return (FALSE);
    }
  printf ("%s\n", buf);
  return (TRUE);
}


static void usage (void)
{
  puts ("Usage: which <command>...");
  if (_osmode == OS2_MODE)
    puts ("       which -d <dll>...");
  exit (1);
}


int main (int argc, char *argv[])
{
  int i, ret_code;
  const char *path;

  ret_code = 0;
  if (argc < 2)
    usage ();
  if (_osmode == OS2_MODE && strcmp (argv[1], "-d") == 0)
    {
      if (argc < 3)
        usage ();
      for (i = 2; i < argc; ++i)
        if (!find_dll (argv[i]))
          ret_code = 2;
    }
  else
    {
      path = getenv ("PATH");
      for (i = 1; i < argc; ++i)
        if (!find_prog (argv[i], path))
          ret_code = 2;
    }
  return (ret_code);
}
