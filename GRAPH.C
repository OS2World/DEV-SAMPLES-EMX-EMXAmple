/* graph.c (emx+gcc) */

/* Test graphics library */

/* Usage: graph */

/* Keyboard control: */

/*   RETURN     Move to next test                               */
/*   ESC        Quit                                            */
/*   SPACE      Pause (type SPACE to continue or ESC to quit)   */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <getopt.h>
#include <limits.h>
#include <conio.h>
#include <graph.h>

#define FALSE 0
#define TRUE  1

static int pal_mode;
static int wait_flag;

static char pal_1[3*255*2];
static int pal_1_idx;

static char pal_2[3*255];
static int color_2[3];
static int dir_2[3];

static char pal[3*256];

/* Return a random number between 0 and n-1.  If n is not a power of two, */
/* the return value is slightly less random than expected at first sight. */
/* Better results can be achieved by discarding values returned by rand() */
/* which are greater than or equal to n * ((RAND_MAX+1) / n).  Of course, */
/* this doesn't matter with this program.                                 */

#define RND(n) (rand () % (n))

#define SLEEP2_MIN 500

static unsigned loops_per_second;
static unsigned loops;
static unsigned volatile counter;
static jmp_buf alarm_jmp;

static void alarm_handler (int sig)
{
  longjmp (alarm_jmp, 1);
}

static void init_waste_time (void)
{
  loops_per_second = ULONG_MAX;
  if (setjmp (alarm_jmp) != 0)
    loops_per_second = counter;
  else
    {
      counter = 0;
      signal (SIGALRM, alarm_handler);
      alarm (1);
      for (counter = 0; counter < loops_per_second; ++counter)
        ;
    }
}


static void waste_time (unsigned millisec)
{
  if (millisec >= SLEEP2_MIN)
    _sleep2 (millisec);
  else
    {
      loops = (loops_per_second * millisec) / 1000;
      for (counter = 0; counter < loops; ++counter)
        ;
    }
}


static void kchar (int c)
{
  if (c == 0x1b)
    {
      g_mode (G_MODE_OFF);
      exit (0);
    }
  else if (c == ' ')
    {
      c = getch ();
      if (c == 0x1b)
        {
          g_mode (G_MODE_OFF);
          exit (0);
        }
    }
}


static void kwait (void)
{
  int c;

  do
    {
      c = getch ();
      kchar (c);
    } while (c == ' ');
}


static int khit_counter = 0;

static int khit (void)
{
  int c;

  ++khit_counter;
  if (khit_counter < 8)
    return (0);
  khit_counter = 0;
  c = _read_kbd (0, 0, 0);
  if (c == -1)
    return (0);
  kchar (c);
  return (c != ' ');
}


static void test_1 (void)
{
  int x, y;

  for (x = 0; x < g_xsize; ++x)
    {
      g_lock ();
      if (g_colors < 256)
        for (y = 0; y < g_ysize; ++y)
          g_set (x, y, (x+y) / 8);
      else
        for (y = 0; y < g_ysize; ++y)
          g_set (x, y, x+y);
      g_unlock ();
    }
  kwait ();
}


static void test_4 (void)
{
  int c1, c2, c3, c4;
  int x, y;

  c1 = G_BLUE; c2 = G_GREEN; c3 = G_CYAN; c4 = G_RED;
  g_clear (G_BLACK);
  for (x = 0; x < g_xsize; x += 5)
    g_line (x, 0, g_xsize/2, g_ysize/2, c1);
  for (y = 0; y < g_ysize; y += 5)
    g_line (g_xsize-1, y, g_xsize/2, g_ysize/2, c2);
  for (x = g_xsize-1; x >= 0; x -= 5)
    g_line (x, g_ysize-1, g_xsize/2, g_ysize/2, c3);
  for (y = g_ysize-1; y >= 0; y -= 5)
    g_line (0, y, g_xsize/2, g_ysize/2, c4);
  kwait ();
}


static void test_5 (void)
{
  int x, y;

  g_clear (G_BLACK);
  for (y = 0; y < g_ysize; ++y)
    g_line (0, 0, g_xsize-1, y, y);
  for (x = g_xsize-1; x >= 0; --x)
    g_line (0, 0, x, g_ysize-1, x);
  kwait ();
}


static void test_6 (void)
{
  g_clear (G_BLACK);
  do
    {
      g_line (RND (g_xsize), RND (g_ysize), RND (g_xsize), RND (g_ysize),
              RND (g_colors));
    } while (!khit ());
}


static void test_7 (void)
{
  int y;

  g_clear (G_BLACK);
  for (y = 0; y*3 < g_xsize && y < g_ysize; ++y)
    g_hline (y, y, y*3, G_WHITE);
  kwait ();
}


static void test_8 (void)
{
  int x;

  g_clear (G_BLACK);
  for (x = 0; x*3 < g_ysize && x < g_xsize; ++x)
    g_vline (x, x, x*3, G_WHITE);
  kwait ();
}


static void test_10 (void)
{
  g_clear (G_BLACK);
  do
    {
      g_box (RND (g_xsize), RND (g_ysize),
             RND (g_xsize), RND (g_ysize),
             RND (g_colors), G_OUTLINE);
    } while (!khit ());
}


static void test_11 (void)
{
  g_clear (G_BLACK);
  do
    {
      g_box (RND (g_xsize), RND (g_ysize),
             RND (g_xsize), RND (g_ysize),
             RND (g_colors), G_FILL);
    } while (!khit ());
}


static void test_19 (void)
{
  int mx, my, rx, ry, fill, cn;

  for (fill = 0; fill <= 1; ++fill)
    {
      g_clear (G_BLACK);
      if (fill == 0)
        cn = g_colors-1;
      else
        cn = g_colors;
      do
        {
          mx = RND (g_xsize);
          my = RND (g_ysize);
          rx = 20 + RND (g_xsize/2);
          ry = 10 + RND (g_ysize/2);
          if (mx+rx < g_xsize && mx-rx > 0 && my+ry < g_ysize && my-ry > 0)
            g_ellipse (mx, my, rx, ry, 1 + RND (cn), fill);
        } while (!khit ());
    }
}


#define MAX_VERT 20

static void test_23 (void)
{
  int i, n, x[MAX_VERT], y[MAX_VERT];

  g_clear (G_BLACK);
  do
    {
      n = 0;
      for (i = 0; i < 2*(MAX_VERT-3); ++i)
        n += RND (2);
      n = 3 + abs (n - (MAX_VERT-3));
      for (i = 0; i < n; ++i)
        {
          x[i] = RND (g_xsize);
          y[i] = RND (g_ysize);
        }
      g_polygon (x, y, n, 1 + RND (g_colors-1), G_FILL);
    } while (!khit ());
}


static void init_pal_1 (void)
{
  int i, j;
  unsigned char func[255];

  for (j = 0; j <= 127; ++j)
    {
      func[j] = (char)(j/2);
      func[254-j] = (char)(j/2);
    }
  for (i = 0; i < 255; ++i)
    {
      pal_1[3*i+0] = func[(i+0*85)%255];
      pal_1[3*i+1] = func[(i+1*85)%255];
      pal_1[3*i+2] = func[(i+2*85)%255];
    }
  memcpy (pal_1+3*255, pal_1, 3*255);
  pal_1_idx = 1;
}


static void make_pal_1 (void)
{
  memcpy (pal+3, pal_1 + pal_1_idx * 3, 3*255);
  ++pal_1_idx;
  if (pal_1_idx > 254)
    pal_1_idx -= 255;
}


static void init_pal_2 (void)
{
  int i;

  memset (pal_2, 0, 3*255);
  for (i = 0; i < 3; ++i)
    {
      dir_2[i] = 1;
      color_2[i] = 0;
    }
}


static void make_pal_2 (void)
{
  int d, i;

  memcpy (pal+3, pal_2, 3*255);
  memmove (pal_2, pal_2+3, 3*254);
  d = 3 * 254;
  pal_2[d+0] = (char)color_2[0];
  pal_2[d+1] = (char)color_2[1];
  pal_2[d+2] = (char)color_2[2];
  for (i = 0; i < 3; ++i)
    {
      color_2[i] += dir_2[i];
      if (color_2[i] < 0 || color_2[i] > 63)
        {
          dir_2[i] = -dir_2[i];
          color_2[i] += dir_2[i];
        }
      else
        break;
    }
}


static void set_pal (void)
{
  g_vgapal (pal, 0, 256, wait_flag);
}


static void make_pal (void)
{
  switch (pal_mode)
    {
    case 1:
      make_pal_1 ();
      set_pal ();
      break;
    case 2:
      make_pal_2 ();
      set_pal ();
      break;
    }
}


static void pal_demo (unsigned millisec)
{
  while (!khit ())
    {
      make_pal ();
      waste_time (millisec);
    }
}


static void demo_1 (unsigned millisec)
{
  int a, color, h, k, m, n, r, s, w, x1, x2, xv1, xv2, y1, y2, yv1, yv2, z;

  w = g_xsize / 2;
  h = g_ysize / 2;
  if (g_ysize < w)
    z = g_ysize;
  else
    z = w;
  k = (z-10)/2;
  r = z / 30;
  s = r / 2;
  m = 20;
  for (;;)
    {
      g_clear (G_BLACK);
      n = 3 * (g_ysize / 2 + RND (g_ysize));
      x1 = 1 + RND (k-1);
      y1 = 1 + RND (k-1);
      x2 = 1 + RND (k-1);
      y2 = 1 + RND (k-1);
      a = 0;
      for (;;)
        {
          if (khit ())
            return;
          if (a <= 0)
            {
              xv1 = RND (r) - s;
              yv1 = RND (r) - s;
              xv2 = RND (r) - s;
              yv2 = RND (r) - s;
              a = 1 + RND (m -1);
              color = 1 + RND (g_colors - 1);
            }
          g_lock ();
          g_line (w+2*x1, h-y1, w+2*x2, h-y2, color);
          g_line (w-2*y1, h+x1, w-2*y2, h+x2, color);
          g_line (w-2*x1, h-y1, w-2*x2, h-y2, color);
          g_line (w-2*y1, h-x1, w-2*y2, h-x2, color);
          g_line (w-2*x1, h+y1, w-2*x2, h+y2, color);
          g_line (w+2*y1, h-x1, w+2*y2, h-x2, color);
          g_line (w+2*x1, h+y1, w+2*x2, h+y2, color);
          g_line (w+2*y1, h+x1, w+2*y2, h+x2, color);
          g_unlock ();
          if (n == 1)
            break;
          --a;
          if (n > 0)
            --n;
          waste_time (millisec); /* This demo looks better when slowed down */
          x1 = (x1+xv1) % k;
          y1 = (y1+yv1) % k;
          x2 = (x2+xv2) % k;
          y2 = (y2+yv2) % k;
        }
    }
}


static void demo_2 (unsigned millisec)
{
  int i, x, y, c, d, k, n, m;
  static int inc_x[4] = {1, 0, -1,  0};
  static int inc_y[4] = {0, 1,  0, -1};

  c = 1; d = 0; n = 16; m = 1;
  x = 0 - inc_x[d];
  y = 0 - inc_y[d];
  k = n;
  for (i = 0; i < 255; ++i)
    {
      x += inc_x[d]; y += inc_y[d];
      g_box (x*20, y*12, (x+1)*20-1, (y+1)*12-1, c, G_FILL);
      if (k <= 1)
        {
          d = (d+1) % 4;
          --m;
          if (m == 0)
            {
              --n; m = 2;
            }
          k = n;
        }
      else
        --k;
      ++c;
    }
  pal_demo (millisec);
}


static void demo_3 (unsigned millisec)
{
  int i;

  for (i = 0; i < 256; ++i)
    g_vline (i, 0, g_ysize-1, i);
  pal_demo (millisec);
}


static void usage (void)
{
  fputs ("Usage: graph [-d#] [-m#] [-p#] [-s#] [-w]\n\n", stderr);
  fputs ("-d#    Select demo mode [0]:\n", stderr);
  fputs ("         -d0  test mode\n", stderr);
  fputs ("         -d1  kaleidoscope demo\n", stderr);
  fputs ("         -d2  palette demo (spiral)\n", stderr);
  fputs ("         -d3  palette demo (band)\n", stderr);
  fputs ("-m#    Select graphics mode [26]:\n", stderr);
  fputs ("         -m16 EGA 640x200, 16 colors\n", stderr);
  fputs ("         -m17 EGA 640x350, 16 colors\n", stderr);
  fputs ("         -m24 VGA 640x480, 16 colors\n", stderr);
  fputs ("         -m26 VGA 320x200, 256 colors\n", stderr);
  fputs ("-p#    Select palette [0]:\n", stderr);
  fputs ("         -p0  default palette\n", stderr);
  fputs ("         -p1  smooth sequence of hues\n", stderr);
  fputs ("         -p2  smooth sequence of all colors\n", stderr);
  fputs ("-s#    Set delay (# milliseconds) for demo modes [6]\n", stderr);
  fputs ("-w     Wait for vertical retrace when modifying palette\n", stderr);
  exit (1);
}


int main (int argc, char *argv[])
{
  int c, demo_mode, delay;
  int graph_mode;
  char *p;

  demo_mode = 0; pal_mode = 0; delay = 6; wait_flag = FALSE;
  graph_mode = G_MODE_VGA_L;
  while ((c = getopt (argc, argv, "d:m:p:s:w")) != EOF)
    switch (c)
      {
      case 'd':
        errno = 0;
        demo_mode = strtol (optarg, &p, 0);
        if (errno != 0 || *p != 0 || demo_mode < 0 || demo_mode > 3)
          usage ();
        break;
      case 'm':
        errno = 0;
        graph_mode = strtol (optarg, &p, 0);
        if (errno != 0 || *p != 0 || graph_mode <= 0)
          usage ();
        break;
      case 'p':
        errno = 0;
        pal_mode = strtol (optarg, &p, 0);
        if (errno != 0 || pal_mode < 1 || pal_mode > 2)
          usage ();
        break;
      case 's':
        errno = 0;
        delay = strtol (optarg, &p, 0);
        if (errno != 0 || *p != 0)
          usage ();
        break;
      case 'w':
        wait_flag = TRUE;
        break;
      default:
        usage ();
      }
  if (optind < argc)
    usage ();
  memset (pal, 0, 3*256);
  switch (pal_mode)
    {
    case 1:
      init_pal_1 ();
      break;
    case 2:
      switch (demo_mode)
        {
        case 0:
          fputs ("-p2 should not be used with -d0\n", stderr);
          exit (1);
        }
      init_pal_2 ();
      break;
    default:
      switch (demo_mode)
        {
        case 2:
        case 3:
          fprintf (stderr, "-d%d should not be used with -p0\n", demo_mode);
          exit (1);
        }
    }
  switch (demo_mode)
    {
    case 1:
    case 2:
    case 3:
      if (delay < SLEEP2_MIN)
        {
          puts ("Please wait..."); fflush (stdout);
          init_waste_time ();
        }
      break;
    }
  if (!g_mode (graph_mode))
    {
      fputs ("Cannot switch to graphics mode\n", stderr);
      return (1);
    }
  make_pal ();
  switch (demo_mode)
    {
    case 0:
      test_1 ();
      test_4 ();
      test_5 ();
      test_6 ();
      test_7 ();
      test_8 ();
      test_10 ();
      test_11 ();
      test_19 ();
      test_23 ();
      demo_1 (0);
      break;
    case 1:
      demo_1 (delay);
      break;
    case 2:
      demo_2 (delay);
      break;
    case 3:
      demo_3 (delay);
      break;
    }
  g_mode (G_MODE_OFF);
  return (0);
}
