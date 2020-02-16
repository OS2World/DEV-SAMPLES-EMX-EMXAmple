/* regexx.c (emx+gcc) -- Copyright (c) 1994 by Eberhard Mattes */

#include <os2emx.h>
#include <stdlib.h>
#include <string.h>
#include <regexp.h>


static void regfun (PCSZ name)
{
  RexxRegisterFunctionDll (name, "REGEXX", name);
}


ULONG regexx_loadfuncs (PCSZ name, LONG argc, const RXSTRING *argv,
                        PCSZ queuename, PRXSTRING retstr)
{
  retstr->strlength = 0;
  if (argc != 0)
    return (1);
  regfun("REGEXXSTART");
  regfun("REGEXXEND");
  regfun("REGEXXSUBSTR");
  regfun("REGEXXREPLACEMATCH");
  regfun("REGEXXREPLACEFIRST");
  regfun("REGEXXREPLACEALL");
  return (0);
}


void regerror (const char *s)
{
}


static char *rxstr_alloc (void)
{
  PVOID p;

  DosAllocMem (&p, 0xf000, PAG_READ | PAG_WRITE | PAG_COMMIT);
  return (p);
}


/* RegexxStart(REGEXP, STRING)

   Find REGEXP in STRING and return the start position of the match.

   Return values:
     0          No match
     1, 2, ...  Position in STRING of the start of the first match */

ULONG regexx_start (PCSZ name, LONG argc, const RXSTRING *argv,
                    PCSZ queuename, PRXSTRING retstr)
{
  regexp *r;
  char *s;
  int n;

  retstr->strlength = 0;
  if (argc != 2 || RXNULLSTRING (argv[0]) || RXNULLSTRING (argv[1]))
    return (1);

  r = regcomp (RXSTRPTR (argv[0]));
  if (r == NULL)
    return (1);
  else
    {
      s = RXSTRPTR (argv[1]);
      if (regexec (r, s) == 0)
        n = 0;
      else
        n = (r->startp[0] - s) + 1;
      free (r);
    }
  _itoa (n, retstr->strptr, 10);
  retstr->strlength = strlen (retstr->strptr);
  return (0);
}


/* RegexxEnd(REGEXP, STRING)

   Find REGEXP in STRING and return the end position of the match.

   Return values:
     0          No match
     1, 2, ...  Position in STRING of the end of the first match */

ULONG regexx_end (PCSZ name, LONG argc, const RXSTRING *argv,
                  PCSZ queuename, PRXSTRING retstr)
{
  regexp *r;
  char *s;
  int n;

  retstr->strlength = 0;
  if (argc != 2 || RXNULLSTRING (argv[0]) || RXNULLSTRING (argv[1]))
    return (1);

  r = regcomp (RXSTRPTR (argv[0]));
  if (r == NULL)
    return (1);
  else
    {
      s = RXSTRPTR (argv[1]);
      if (regexec (r, s) == 0)
        n = 0;
      else
        n = r->endp[0] - s;
      free (r);
    }
  _itoa (n, retstr->strptr, 10);
  retstr->strlength = strlen (retstr->strptr);
  return (0);
}


/* RegexxSubstr(REGEXP, STRING)

   Find REGEXP in STRING and return the matching substring.  Return
   the empty string if there is no match. */

ULONG regexx_substr (PCSZ name, LONG argc, const RXSTRING *argv,
                     PCSZ queuename, PRXSTRING retstr)
{
  regexp *r;
  char *s;
  int n;
  size_t len;

  retstr->strlength = 0;
  if (argc != 2 || RXNULLSTRING (argv[0]) || RXNULLSTRING (argv[1]))
    return (1);

  r = regcomp (RXSTRPTR (argv[0]));
  if (r == NULL)
    return (1);
  else
    {
      s = RXSTRPTR (argv[1]);
      if (regexec (r, s) != 0)
        {
          len = r->endp[0] - r->startp[0];
          if (len > RXAUTOBUFLEN)
            retstr->strptr = rxstr_alloc ();
          memcpy (retstr->strptr, r->startp[0], len);
          retstr->strlength = len;
        }
      free (r);
    }
  return (0);
}


/* RegexxReplaceMatch(REGEXP, REPLACEMENT, STRING)

   Find REGEXP in STRING, replace the matching substring with
   REPLACEMENT, and return that substring. */

ULONG regexx_replacematch (PCSZ name, LONG argc, const RXSTRING *argv,
                           PCSZ queuename, PRXSTRING retstr)
{
  regexp *r;
  char *result;
  size_t len;

  retstr->strlength = 0;
  if (argc != 3 || RXNULLSTRING (argv[0]) || RXNULLSTRING (argv[1])
      || RXNULLSTRING (argv[2]))
    return (1);

  r = regcomp (RXSTRPTR (argv[0]));
  if (r == NULL)
    return (1);

  if (regexec (r, RXSTRPTR (argv[2])) == 0)
    retstr->strlength = 0;
  else
    {
      result = rxstr_alloc ();
      regsub (r, RXSTRPTR (argv[1]), result);
      len = strlen (result);
      if (len <= RXAUTOBUFLEN)
        {
          memcpy (retstr->strptr, result, len);
          retstr->strlength = len;
          DosFreeMem (result);
        }
      else
        MAKERXSTRING (*retstr, result, len);
    }
  free (r);
  return (0);
}


/* RegexxReplaceFirst(REGEXP, REPLACEMENT, STRING)

   Replace the first match of REGEXP in STRING with REPLACEMENT and
   return the resulting string. */

ULONG regexx_replacefirst (PCSZ name, LONG argc, const RXSTRING *argv,
                           PCSZ queuename, PRXSTRING retstr)
{
  regexp *r;

  retstr->strlength = 0;
  if (argc != 3 || RXNULLSTRING (argv[0]) || RXNULLSTRING (argv[1])
      || RXNULLSTRING (argv[2]))
    return (1);

  r = regcomp (RXSTRPTR (argv[0]));
  if (r == NULL)
    return (1);

  if (regexec (r, RXSTRPTR (argv[2])) == 0)
    {
      if (RXSTRLEN (argv[2]) > RXAUTOBUFLEN)
        retstr->strptr =  rxstr_alloc ();
      memcpy (retstr->strptr, RXSTRPTR (argv[2]), RXSTRLEN (argv[2]));
      retstr->strlength = RXSTRLEN (argv[2]);
    }
  else
    {
      char *result;
      size_t pos, len;

      result = rxstr_alloc ();
      pos = 0;
      len = (PCH)r->startp[0] - RXSTRPTR (argv[2]);
      memcpy (result + pos, RXSTRPTR (argv[2]), len); pos += len;
      regsub (r, RXSTRPTR (argv[1]), result + pos);
      pos += strlen (result + pos);
      len = RXSTRLEN (argv[2]) - ((PCH)r->endp[0] - RXSTRPTR (argv[2]));
      memcpy (result + pos, r->endp[0], len); pos += len;
      if (pos <= RXAUTOBUFLEN)
        {
          memcpy (retstr->strptr, result, pos);
          retstr->strlength = pos;
          DosFreeMem (result);
        }
      else
        MAKERXSTRING (*retstr, result, pos);
    }
  free (r);
  return (0);
}


/* RegexxReplaceAll(REGEXP, REPLACEMENT, STRING)

   Replace the all matches of REGEXP in STRING with REPLACEMENT and
   return the resulting string. */

ULONG regexx_replaceall (PCSZ name, LONG argc, const RXSTRING *argv,
                         PCSZ queuename, PRXSTRING retstr)
{
  regexp *r;
  char *result, *s;
  size_t pos, len;

  retstr->strlength = 0;
  if (argc != 3 || RXNULLSTRING (argv[0]) || RXNULLSTRING (argv[1])
      || RXNULLSTRING (argv[2]))
    return (1);

  r = regcomp (RXSTRPTR (argv[0]));
  if (r == NULL)
    return (1);

  result = rxstr_alloc ();
  pos = 0;
  s = RXSTRPTR (argv[2]);
  while (regexec (r, s) != 0)
    {
      len = r->startp[0] - s;
      memcpy (result + pos, s, len); pos += len;
      regsub (r, RXSTRPTR (argv[1]), result + pos);
      pos += strlen (result + pos);
      s = r->endp[0];
    }
  len = strlen (s);
  memcpy (result + pos, s, len); pos += len;
  if (pos <= RXAUTOBUFLEN)
    {
      memcpy (retstr->strptr, result, pos);
      retstr->strlength = pos;
      DosFreeMem (result);
    }
  else
    MAKERXSTRING (*retstr, result, pos);

  free (r);
  return (0);
}
