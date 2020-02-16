/* wm_hello.c (emx+gcc) */

#include <stdio.h>
#include <stdlib.h>
#include <sys/winmgr.h>

int main (void)
{
  wm_handle my_first_window;                           /* Window handle      */

  if (!wm_init (10))                                   /* At most 10 windows */
    goto failure;                                      /* ...failed          */
  my_first_window = wm_create (20, 10, 59, 14,         /* Window corners     */
                               2,                      /* Border style       */
                               BW_NORMAL|INTENSITY,    /* Border attributes  */
                               BW_NORMAL);             /* Default attributes */
  if (my_first_window == NULL)
    goto failure;                                      /* ...failed          */
  wm_printf (my_first_window, "Hello world!\n");
  wm_open (my_first_window);                           /* Open the window    */
  sleep (2);
  wm_close_all ();                                     /* Close all windows  */
  wm_exit ();                                          /* End window manager */
  return (0);                                          /* Done               */

failure:
  fprintf (stderr, "A window manager function call failed, sorry.\n");
  return (1);
}
