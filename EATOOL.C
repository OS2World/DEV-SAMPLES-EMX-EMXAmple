/* eatool.c (emx+gcc) -- Copyright (c) 1992-1993 by Eberhard Mattes */

#define INCL_DOSFILEMGR
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <io.h>
#include <sys/nls.h>
#include <sys/ead.h>

#define FALSE 0
#define TRUE  1

enum actions
{
  ACTION_NONE,
  ACTION_ADD,
  ACTION_DISCARD,
  ACTION_LIST,
  ACTION_GET,
  ACTION_MERGE,
  ACTION_PUT,
  ACTION_REMOVE
};

static int debugging = FALSE;
static enum actions action = ACTION_NONE;
static _ead empty_ead = NULL;


static void do_add (const char *path, const char *name, const char *value)
{
  int i, size;
  _ead ead;
  char *buf, *uname;

  uname = strdup (name);
  if (uname == NULL)
    {
      fprintf (stderr, "Out of memory\n");
      exit (2);
    }
  _nls_strupr (uname);
  ead = _ead_create ();
  if (ead == NULL)
    {
      perror (NULL);
      exit (2);
    }
  size = strlen (value);
  buf = malloc (size + 4);
  if (buf == NULL)
    {
      fprintf (stderr, "Out of memory\n");
      exit (2);
    }
  ((USHORT *)buf)[0] = EAT_ASCII;
  ((USHORT *)buf)[1] = size;
  memcpy (buf+4, value, size);
  if (_ead_add (ead, uname, 0, buf, size+4) < 0)
    {
      perror (path);
      exit (2);
    }
  if (strcmp (path, "-") == 0)
    i = _ead_write (ead, NULL, fileno (stdout), _EAD_MERGE);
  else
    i = _ead_write (ead, path, 0, _EAD_MERGE);
  if (i < 0)
    perror (path);
  free (buf); free (uname);
  _ead_destroy (ead);
}


static void do_remove (const char *path, const char *name)
{
  _ead ead;
  int i;
  char *uname;

  ead = _ead_create ();
  if (ead == NULL)
    {
      perror (NULL);
      exit (2);
    }
  uname = strdup (name);
  if (uname == NULL)
    {
      fprintf (stderr, "Out of memory\n");
      exit (2);
    }
  _nls_strupr (uname);
  if (_ead_read (ead, path, 0, 0) < 0)
    perror (path);
  i = _ead_find (ead, uname);
  if (i < 0)
    {
      perror (path);
      exit (2);
    }
  if (_ead_delete (ead, i) < 0)
    {
      perror ("_ead_delete");
      exit (2);
    }
  if (_ead_write (ead, path, 0, 0) < 0)
    perror (path);
  free (uname);
  _ead_destroy (ead);
}


static void do_discard (const char *path)
{
  int i;

  if (empty_ead == NULL)
    {
      empty_ead = _ead_create ();
      if (empty_ead == NULL)
        {
          perror (NULL);
          exit (2);
        }
    }
  if (strcmp (path, "-") == 0)
    i = _ead_write (empty_ead, NULL, fileno (stdout), 0);
  else
    i = _ead_write (empty_ead, path, 0, 0);
  if (i < 0)
    perror (path);
}


static void do_list (const char *path)
{
  _ead ead;
  int i, n, nlen, vsize;
  const char *name, *value;

  ead = _ead_create ();
  if (ead == NULL)
    {
      perror (NULL);
      exit (2);
    }
  if (strcmp (path, "-") == 0)
    i = _ead_read (ead, NULL, fileno (stdin), 0);
  else
    i = _ead_read (ead, path, 0, 0);
  if (i < 0)
    perror (path);
  else
    {
      if (debugging)
        printf ("%s (%d %d %d):\n", path,
                _ead_name_len (ead, 0), _ead_value_size (ead, 0),
                _ead_fea2list_size (ead));
      else
        printf ("%s:\n", path);
      _ead_sort (ead);
      n = _ead_count (ead);
      for (i = 1; i <= n; ++i)
        {
          name = _ead_get_name (ead, i);
          if (name == NULL)
            perror (path);
          else
            {
              vsize = _ead_value_size (ead, i);
              if (vsize < 0) perror (path);
              if (debugging)
                {
                  nlen = _ead_name_len (ead, i);
                  if (nlen < 0) perror (path);
                  printf ("  %s (%d): ", name, nlen);
                }
              else
                printf ("  %s: ", name);
              if (vsize >= 4)
                {
                  value = _ead_get_value (ead, i);
                  if (value == NULL)
                    perror (path);
                  else
                    switch (*(USHORT *)value)
                      {
                      case EAT_ASCII:
                        printf ("\"");
                        fwrite (value+4, vsize-4, 1, stdout);
                        printf ("\"\n");
                        break;
                      case EAT_BINARY:
                        printf ("%d bytes of binary data\n", vsize);
                        break;
                      case EAT_BITMAP:
                        printf ("%d bytes of bitmap data\n", vsize);
                        break;
                      case EAT_METAFILE:
                        printf ("%d bytes of metafile data\n", vsize);
                        break;
                      case EAT_ICON:
                        printf ("%d bytes of icon data\n", vsize);
                        break;
                      case EAT_EA:
                        printf ("%d bytes of associated data\n", vsize);
                        break;
                      case EAT_MVMT:
                      case EAT_MVST:
                      case EAT_ASN1:
                        printf ("%d bytes of multivalue data\n", vsize);
                        break;
                      default:
                        printf ("%d bytes\n", vsize);
                        break;
                      }
                }
              else
                printf ("%d bytes\n", vsize);
            }
        }
      _ead_destroy (ead);
    }
}


static void do_put (const char *path, const char *hold)
{
  _ead ead;
  int i;
  PFEALIST pfealist;
  const FEA2LIST *pfea2list;
  FILE *hold_file;

  ead = _ead_create ();
  if (ead == NULL)
    {
      perror (NULL);
      exit (2);
    }
  if (strcmp (path, "-") == 0)
    i = _ead_read (ead, NULL, fileno (stdin), 0);
  else
    i = _ead_read (ead, path, 0, 0);
  if (i < 0)
    perror (path);
  else
    {
      hold_file = fopen (hold, "wb");
      if (hold_file == NULL)
        {
          perror (hold);
          exit (2);
        }
      pfea2list = _ead_get_fea2list (ead);
      if (pfea2list != NULL)
        {
          pfealist = _ead_fea2list_to_fealist (pfea2list);
          fwrite (pfealist, pfealist->cbList, 1, hold_file);
          free (pfealist);
        }
      if (fflush (hold_file) != 0 || fclose (hold_file) != 0)
        {
          perror (hold);
          exit (2);
        }
      _ead_destroy (ead);
    }
}


static void do_get (const char *path, const char *hold, int merge)
{
  int i, wflag;
  FILE *f;
  long size;
  PFEALIST pfealist;
  PFEA2LIST pfea2list;
  _ead ead;

  ead = _ead_create ();
  if (ead == NULL)
    {
      perror (NULL);
      exit (2);
    }
  f = fopen (hold, "rb");
  if (f == NULL)
    {
      perror (hold);
      exit (2);
    }
  size = filelength (fileno (f));
  if (size == -1)
    {
      perror (hold);
      exit (2);
    }
  if (size != 0)
    {
      pfealist = malloc (size);
      if (pfealist == NULL)
        {
          fprintf (stderr, "Out of memory\n");
          exit (2);
        }
      if (fread (pfealist, size, 1, f) != 1)
        {
          if (ferror (f))
            perror (hold);
          else
            fprintf (stderr, "%s: end of file reached\n", hold);
          exit (2);
        }
      free (pfealist);
      pfea2list = _ead_fealist_to_fea2list (pfealist);
      if (pfea2list == NULL)
        {
          perror (NULL);
          exit (2);
        }
      if (_ead_use_fea2list (ead, pfea2list) != 0)
        {
          perror (NULL);
          exit (2);
        }
      free (pfea2list);
    }
  fclose (f);
  wflag = (merge ? _EAD_MERGE : 0);
  if (strcmp (path, "-") == 0)
    i = _ead_write (ead, NULL, fileno (stdout), wflag);
  else
    i = _ead_write (ead, path, 0, wflag);
  if (i < 0)
    {
      perror (path);
      exit (2);
    }
  _ead_destroy (ead);
}


static void usage (void)
{
  fputs ("Usage: eatool -g <datafile> <holdfile>\n", stderr);
  fputs ("       eatool -m <datafile> <holdfile>\n", stderr);
  fputs ("       eatool -p <datafile> <holdfile>\n", stderr);
  fputs ("       eatool -d <files>\n", stderr);
  fputs ("       eatool -l [-z] <files>\n", stderr);
  fputs ("       eatool -a <file> <name> <value>\n", stderr);
  fputs ("       eatool -r <file> <name>\n", stderr);
  fputs ("Options:\n", stderr);
  fputs ("  -a   Add extended attribute\n", stderr);
  fputs ("  -d   Discard extended attributes\n", stderr);
  fputs ("  -g   Get extended attributes from holdfile\n", stderr);
  fputs ("  -m   Merge extended attributes from holdfile\n", stderr);
  fputs ("  -l   List extended attributes\n", stderr);
  fputs ("  -p   Put extended attributes into holdfile\n", stderr);
  fputs ("  -r   Remove extended attribute\n", stderr);
  fputs ("  -z   Turn on debugging output\n", stderr);
  exit (1);
}


static void set_action (enum actions a)
{
  if (action != ACTION_NONE)
    usage ();
  action = a;
}


static void apply (int argc, char *argv[], void (*function)(const char *path))
{
  int i, j;
  char **list;

  for (i = optind; i < argc; ++i)
    {
      list = _fnexplode (argv[i]);
      if (list == NULL)
        function (argv[i]);
      else
        {
          for (j = 0; list[j] != NULL; ++j)
            function (list[j]);
          _fnexplodefree (list);
        }
    }
}


int main (int argc, char *argv[])
{
  int c;

  opterr = FALSE;
  while ((c = getopt (argc, argv, "adglmprz")) != EOF)
    {
      switch (c)
        {
        case 'a':
          set_action (ACTION_ADD);
          break;
        case 'd':
          set_action (ACTION_DISCARD);
          break;
        case 'g':
          set_action (ACTION_GET);
          break;
        case 'm':
          set_action (ACTION_MERGE);
          break;
        case 'l':
          set_action (ACTION_LIST);
          break;
        case 'p':
          set_action (ACTION_PUT);
          break;
        case 'r':
          set_action (ACTION_REMOVE);
          break;
        case 'z':
          debugging = TRUE;
          break;
        default:
          usage ();
        }
    }
  _nls_init ();
  switch (action)
    {
    case ACTION_ADD:
      if (argc - optind != 3)
        usage ();
      do_add (argv[optind+0], argv[optind+1], argv[optind+2]);
      break;
    case ACTION_DISCARD:
      apply (argc, argv, do_discard);
      break;
    case ACTION_GET:
    case ACTION_MERGE:
      if (argc - optind != 2)
        usage ();
      do_get (argv[optind+0], argv[optind+1], action == ACTION_MERGE);
      break;
    case ACTION_LIST:
      apply (argc, argv, do_list);
      break;
    case ACTION_PUT:
      if (argc - optind != 2)
        usage ();
      do_put (argv[optind+0], argv[optind+1]);
      break;
    case ACTION_REMOVE:
      if (argc - optind != 2)
        usage ();
      do_remove (argv[optind+0], argv[optind+1]);
      break;
    default:
      usage ();
    }
  return (0);
}
