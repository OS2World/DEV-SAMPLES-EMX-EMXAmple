/* calc.c (emx+gcc) -- Copyright (c) 1992-1993 by Eberhard Mattes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <math.h>
#include <ctype.h>

#define NORETURN2 __attribute__ ((noreturn))

#define FALSE 0
#define TRUE  1

#define T_EOF		0
#define T_NAME          1
#define T_NUMBER	2
#define T_LPAR		3
#define T_RPAR		4
#define T_ASSIGN	5
#define T_OPERATOR	6
#define T_EOL           7
#define T_HELP          8
#define T_QUIT          9

struct keyword
{
  char *name;
  int token;
  double (*unary)(double x);
  double (*binary)(double x, double y);
  int u_bp, b_bp;
};

static FILE *parse_f;
static char parse_buffer[512];
static char tokstr[512];
static const unsigned char *parse_ptr;
static int parse_line;
static int parse_cont;
static int interactive;
static int token;
static int calc_errno;
static const struct keyword *tokptr;
static double toknum;
static jmp_buf main_loop;

static double f_add (double x, double y);
static double f_sub (double x, double y);
static double f_mul (double x, double y);
static double f_div (double x, double y);
static double f_shl (double x, double y);
static double f_shr (double x, double y);
static double f_and (double x, double y);
static double f_or (double x, double y);
static double f_eq (double x, double y);
static double f_ne (double x, double y);
static double f_lt (double x, double y);
static double f_le (double x, double y);
static double f_gt (double x, double y);
static double f_ge (double x, double y);
static double f_neg (double x);
static double f_not (double x);
static void syntax (const char *msg) NORETURN2;
static int get_line (void);
static void get_token (int more);
static void check (int t, const char *msg);
static double factor (void);
static double expr (void);
static double expr1 (int min_bp);
static void help (void);


static struct keyword keywords[] =
{
  {"+",         T_OPERATOR, NULL,  f_add,  0, 10},
  {"++",        T_OPERATOR, NULL,  hypot,  0, 10},
  {"-",         T_OPERATOR, f_neg, f_sub, 12, 10},
  {"*",         T_OPERATOR, NULL,  f_mul,  0, 14},
  {"/",         T_OPERATOR, NULL,  f_div,  0, 14},
  {"%",         T_OPERATOR, NULL,  fmod,   0, 14},
  {"=",         T_OPERATOR, NULL,  f_eq,   0,  8},
  {"<>",        T_OPERATOR, NULL,  f_ne,   0,  8},
  {"!=",        T_OPERATOR, NULL,  f_ne,   0,  8},
  {"<",         T_OPERATOR, NULL,  f_lt,   0,  8},
  {"<=",        T_OPERATOR, NULL,  f_le,   0,  8},
  {">",         T_OPERATOR, NULL,  f_gt,   0,  8},
  {">=",        T_OPERATOR, NULL,  f_ge,   0,  8},
  {"^",         T_OPERATOR, NULL,  pow,    0, 21},
  {"**",        T_OPERATOR, NULL,  pow,    0, 21},
  {"<<",        T_OPERATOR, NULL,  f_shl,  0, 16},
  {">>",        T_OPERATOR, NULL,  f_shr,  0, 16},
  {"&",         T_OPERATOR, NULL,  f_and,  0,  4},
  {"|",         T_OPERATOR, NULL,  f_or,   0,  2},
  {"!",         T_OPERATOR, f_not, NULL,   6,  0},
  {"(",         T_LPAR,     NULL,  NULL,   0,  0},
  {")",         T_RPAR,     NULL,  NULL,   0,  0},
  {"or",        T_OPERATOR, NULL,  f_or,   0,  2},
  {"and",       T_OPERATOR, NULL,  f_and,  0,  4},
  {"not",       T_OPERATOR, f_not, NULL,   6,  0},
  {"exp",       T_OPERATOR, exp,   NULL,  18,  0},
  {"log",       T_OPERATOR, log10, NULL,  18,  0},
  {"ln",        T_OPERATOR, log,   NULL,  18,  0},
  {"cos",       T_OPERATOR, cos,   NULL,  18,  0},
  {"sin",       T_OPERATOR, sin,   NULL,  18,  0},
  {"tan",       T_OPERATOR, tan,   NULL,  18,  0},
  {"cosh",      T_OPERATOR, cosh,  NULL,  18,  0},
  {"sinh",      T_OPERATOR, sinh,  NULL,  18,  0},
  {"tanh",      T_OPERATOR, tanh,  NULL,  18,  0},
  {"acos",      T_OPERATOR, acos,  NULL,  18,  0},
  {"asin",      T_OPERATOR, asin,  NULL,  18,  0},
  {"atan",      T_OPERATOR, atan,  NULL,  18,  0},
  {"cbrt",      T_OPERATOR, cbrt,  NULL,  18,  0},
  {"sqrt",      T_OPERATOR, sqrt,  NULL,  18,  0},
  {"ceil",      T_OPERATOR, ceil,  NULL,  18,  0},
  {"floor",     T_OPERATOR, floor, NULL,  18,  0},
  {"round",     T_OPERATOR, rint,  NULL,  18,  0},
  {"trunc",     T_OPERATOR, trunc, NULL,  18,  0},
  {"abs",       T_OPERATOR, fabs,  NULL,  18,  0},
  {"help",      T_HELP,     NULL,  NULL,   0,  0},
  {"quit",      T_QUIT,     NULL,  NULL,   0,  0},
  {NULL,        0,          NULL,  NULL,   0,  0}
};


static void syntax (const char *msg)
{
  fprintf (stderr, "%s (Token %s, line %d)\n", msg, tokstr, parse_line);
  if (interactive)
    longjmp (main_loop, 1);
  else
    exit (2);
}


static int get_line (void)
{
  char *p;

  if (interactive != 0)
    {
      if (parse_cont)
        printf (">> ");
      else
        printf ("%d> ", interactive++);
      fflush (stdout);
    }
  if (fgets (parse_buffer, sizeof (parse_buffer), parse_f) == NULL)
    {
      if (!ferror (parse_f))
	return (FALSE);
      perror ("fgets");
      exit (2);
    }
  p = strchr (parse_buffer, '\n');
  if (p != NULL) *p = 0;
  parse_ptr = parse_buffer;
  ++parse_line; parse_cont = TRUE;
  return (TRUE);
}


static int find_token (const struct keyword *table)
{
  while (table->name != NULL)
    if (strcmp (tokstr, table->name) == 0)
      {
	token = table->token;
	tokptr = table;
	return (TRUE);
      }
    else
      ++table;
  return (FALSE);
}


static int find_prefix (void)
{
  size_t len;
  int j;
  const struct keyword *table;
  
  len = strlen (tokstr); j = 0;
  for (table = keywords; table->name != NULL; ++table)
    if (strlen (table->name) >= len &&
	memcmp (table->name, tokstr, len) == 0)
      ++j;
  return (j);
}


static int find_exact (void)
{
  size_t len;
  int j;
  const struct keyword *table;
  
  len = strlen (tokstr); j = 0;
  for (table = keywords; table->name != NULL; ++table)
    if (strlen (table->name) == len && strcmp (table->name, tokstr) == 0)
      {
	token = table->token;
	tokptr = table;
	++j;
      }
  return (j);
}

static void get_token (int more)
{
  const unsigned char *start;
  char *p;
  int i, j;
  
  for (;;)
    {
      if (*parse_ptr == ' ' || *parse_ptr == '\t')
	++parse_ptr;
      else if (*parse_ptr == 0 || *parse_ptr == '#')
	{
          if (!more)
            {
              strcpy (tokstr, "<<EOL>>");
              token = T_EOL;
              return;
            }
	  if (!get_line ())
	    {
	      strcpy (tokstr, "<<EOF>>");
	      token = T_EOF;
	      return;
	    }
	}
      else
	break;
    }
  if (*parse_ptr >= 0x80 || isalpha (*parse_ptr))
    {
      start = parse_ptr++;
      while (*parse_ptr >= 0x80 || isalnum (*parse_ptr) || *parse_ptr == '_')
	++parse_ptr;
      i = parse_ptr - start;
      if (i+1 > sizeof (tokstr))
        syntax ("Name too long");
      memcpy (tokstr, start, i);
      tokstr[i] = 0;
      if (!find_token (keywords))
	token = T_NAME;
    }
  else if (isdigit (*parse_ptr))
    {
      errno = 0;
      toknum = strtod (parse_ptr, &p);
      if (errno != 0)
	syntax ("Invalid number");
      token = T_NUMBER;
      i = p - (char *)parse_ptr + 1;
      if (i > sizeof (tokstr))
        i = sizeof (tokstr);
      _strncpy (tokstr, parse_ptr, i);
      parse_ptr = p;
    }
  else
    {
      tokstr[0] = *parse_ptr++;
      i = 1;
      for (;;)
	{
	  tokstr[i] = 0;
	  j = find_prefix ();
	  if (j == 0 || *parse_ptr == 0)
	    break;
	  tokstr[i++] = *parse_ptr++;
	}
      while (find_exact () != 1)
	{
	  if (i == 1)
	    syntax ("Invalid character");
	  --i; --parse_ptr;
	  tokstr[i] = 0;
	}
    }
}


static void check (int t, const char *msg)
{
  while (token == T_EOL)
    get_token (TRUE);
  if (token != t)
    {
      char buf[100];

      sprintf (buf, "`%s' expected", msg);
      syntax (buf);
    }
}


static double factor (void)
{
  double x;
  const struct keyword *ptr;

  switch (token)
    {
    case T_NUMBER:
      x = toknum;
      get_token (FALSE);
      return (x);
    case T_NAME:
      syntax ("Variables not yet implemented");
    case T_LPAR:
      get_token (TRUE);
      x = expr ();
      check (T_RPAR, ")");
      get_token (FALSE);
      return (x);
    case T_OPERATOR:
      if (tokptr->unary == NULL)
        syntax ("Unary operator expected");
      ptr = tokptr;
      get_token (TRUE);
      x = expr1 (ptr->u_bp);
      errno = 0;
      x = ptr->unary (x);
      if (errno != 0)
        calc_errno = errno;
      return (x);
    default:
      syntax ("Operand expected");
    }
}


static double expr1 (int min_bp)
{
  double x, y;
  const struct keyword *ptr;

  min_bp &= ~1;
  x = factor ();
  while (token == T_OPERATOR && tokptr->binary != NULL &&
         tokptr->b_bp > min_bp)
    {
      ptr = tokptr;
      get_token (TRUE);
      y = expr1 (ptr->b_bp);
      errno = 0;
      x = ptr->binary (x, y);
      if (errno != 0)
        calc_errno = errno;
    }
  return (x);
}


static double expr (void)
{
  return (expr1 (0));
}


static double f_add (double x, double y)
{
  return (x + y);
}


static double f_sub (double x, double y)
{
  return (x - y);
}


static double f_mul (double x, double y)
{
  return (x * y);
}


static double f_div (double x, double y)
{
  return (x / y);
}


static double f_shl (double x, double y)
{
  return (ldexp (x, (int)y));
}


static double f_shr (double x, double y)
{
  return (ldexp (x, (int)(-y)));
}


static double f_eq (double x, double y)
{
  return (x == y ? 1.0 : 0.0);
}


static double f_ne (double x, double y)
{
  return (x != y ? 1.0 : 0.0);
}


static double f_lt (double x, double y)
{
  return (x < y ? 1.0 : 0.0);
}


static double f_le (double x, double y)
{
  return (x <= y ? 1.0 : 0.0);
}


static double f_gt (double x, double y)
{
  return (x > y ? 1.0 : 0.0);
}


static double f_ge (double x, double y)
{
  return (x >= y ? 1.0 : 0.0);
}


static double f_neg (double x)
{
  return (-x);
}


static double f_and (double x, double y)
{
  return (x != 0.0 && y != 0.0 ? 1.0 : 0.0);
}


static double f_or (double x, double y)
{
  return (x != 0.0 || y != 0.0 ? 1.0 : 0.0);
}


static double f_not (double x)
{
  return (x == 0.0 ? 1.0 : 0.0);
}


static void help (void)
{
  puts ("This is the emx calculator. "
        "Copyright (c) 1992-1993 by Eberhard Mattes.\n");
  puts ("Commands:");
  puts ("  help           display help");
  puts ("  quit           exit the emx calculator");
  puts ("  <exp>          evaluate the expression <exp> and print it\n");
  puts ("Operators: (ordered by precedence):");
  puts ("  ( )            change order of evaluation");
  puts ("  ^ **           compute power");
  puts ("  sin cos tan sinh cosh tanh asin acos atan exp log ln");
  puts ("      cbrt sqrt abs ceil floor round trunc");
  puts ("  << >>          shift left, shift right");
  puts ("  * / %          multiplication, division, modulus");
  puts ("  -              change sign");
  puts ("  + - ++         addition, subtraction, hypot");
  puts ("  = != < <= > >= compare numbers");
  puts ("  ! not          logical not");
  puts ("  & and          logical and");
  puts ("  | or           logical or");
  puts ("All operators but ^ and ** are left-associative");
}


int main (int argc, char *argv[])
{
  double x;

  if (argc == 1)
    {
      parse_f = stdin;
      interactive = 1;
    }
  else if (argc == 2)
    {
      interactive = 0;
      parse_f = fopen (argv[1], "rt");
      if (parse_f == NULL)
        {
          fprintf (stderr, "Cannot open input file %s\n", argv[1]);
          return (2);
        }
    }
  else
    {
      fprintf (stderr, "Usage: calc [filename]\n");
      return (1);
    }
  parse_line = 0;
  if (interactive)
    printf ("Type `help' for help.\n");
  for (;;)
    {
      setjmp (main_loop);
      if (interactive)
        printf ("\n");
      parse_cont = FALSE;
      parse_buffer[0] = 0; parse_ptr = parse_buffer;
      calc_errno = 0;
      get_token (TRUE);
      if (token == T_EOF || token == T_QUIT)
        break;
      else if (token == T_HELP)
        help ();
      else
        {
          x = expr ();
          if (token != T_EOL)
            syntax ("extra characters at end of line");
          if (interactive)
            printf ("==> ");
          printf ("%g", x);
          if (calc_errno != 0)
            printf (" (%s)", strerror (calc_errno));
          printf ("\n");
        }
    }
  return (0);
}
