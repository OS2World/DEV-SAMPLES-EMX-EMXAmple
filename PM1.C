/* pm1.c (emx+gcc) */

#include <stdio.h>
#define INCL_WIN
#include <os2.h>
#include "pm1.h"

static MRESULT EXPENTRY ClientWndProc (HWND hwnd, ULONG msg,
                                       MPARAM mp1, MPARAM mp2)
{
  HPS hps;
  RECTL rcl;

  switch (msg)
    {
    case WM_PAINT:
      hps = WinBeginPaint (hwnd, NULLHANDLE, NULL);
      WinQueryWindowRect (hwnd, &rcl);
      GpiSetColor (hps, CLR_DARKCYAN);
      WinDrawText (hps, -1, "Hello, world!", &rcl, 0, 0,
                   DT_TEXTATTRS | DT_CENTER | DT_VCENTER | DT_ERASERECT);
      WinEndPaint (hps);
      return (0);

    case WM_COMMAND:
      switch (SHORT1FROMMP (mp1))
        {
        case IDM_EXIT:
          WinSendMsg (hwnd, WM_CLOSE, NULL, NULL);
          return ((MRESULT)0);
        }
      break;
    }
  return (WinDefWindowProc (hwnd, msg, mp1, mp2));
}


int main (void)
{
  ULONG flFrameFlags;
  static char szClientClass[] = "pm1.child";
  HAB hab;
  HMQ hmq;
  HWND hwndFrame;
  QMSG qmsg;

  hab = WinInitialize (0);
  hmq = WinCreateMsgQueue (hab, 0);

  WinRegisterClass (hab, szClientClass, ClientWndProc,
                    CS_SIZEREDRAW, 0L);

  flFrameFlags = (FCF_TITLEBAR      | FCF_SYSMENU |
                  FCF_SIZEBORDER    | FCF_MINMAX   |
                  FCF_MENU          | FCF_ACCELTABLE |
                  FCF_SHELLPOSITION | FCF_ICON |
                  FCF_TASKLIST);

  hwndFrame = WinCreateStdWindow (HWND_DESKTOP, WS_VISIBLE,
                                  &flFrameFlags, szClientClass,
                                  NULL, 0L, 0, ID_PM1, NULL);

  while (WinGetMsg (hab, &qmsg, 0L, 0, 0))
    WinDispatchMsg (hab, &qmsg);

  WinDestroyWindow (hwndFrame);
  WinDestroyMsgQueue (hmq);
  WinTerminate (hab);
  return (0);
}
