/* flagdlg.c (emx+gcc) */

/* This is part of the Flag Workplace Shell sample program. */

#define USE_OS2_TOOLKIT_HEADERS

#include "flag.ih"
#include "flag.ph"

#undef SOM_CurrentClass

#define GET_RED(X)      ((UCHAR)((X) >> 16))
#define GET_GREEN(X)    ((UCHAR)((X) >> 8))
#define GET_BLUE(X)     ((UCHAR)((X) >> 0))
#define MAKE_RGB(R,G,B) (((ULONG)(R) << 16) | ((ULONG)(G) << 8) | (B))

static ULONG get_color (HWND hwnd, ULONG idc)
{
  ULONG r, g, b;

  WinSendDlgItemMsg (hwnd, idc + 0, SPBM_QUERYVALUE,
                     (MPARAM)&r, MPFROM2SHORT (0, SPBQ_UPDATEIFVALID));
  WinSendDlgItemMsg (hwnd, idc + 1, SPBM_QUERYVALUE,
                     (MPARAM)&g, MPFROM2SHORT (0, SPBQ_UPDATEIFVALID));
  WinSendDlgItemMsg (hwnd, idc + 2, SPBM_QUERYVALUE,
                     (MPARAM)&b, MPFROM2SHORT (0, SPBQ_UPDATEIFVALID));
  return (MAKE_RGB (r, g, b));
}


static void set_color (HWND hwnd, ULONG idc, ULONG color)
{
  WinSendDlgItemMsg (hwnd, idc + 0, SPBM_SETCURRENTVALUE,
                     MPFROMLONG (GET_RED (color)), MPFROMLONG (0));
  WinSendDlgItemMsg (hwnd, idc + 1, SPBM_SETCURRENTVALUE,
                     MPFROMLONG (GET_GREEN (color)), MPFROMLONG (0));
  WinSendDlgItemMsg (hwnd, idc + 2, SPBM_SETCURRENTVALUE,
                     MPFROMLONG (GET_BLUE (color)), MPFROMLONG (0));
}


static void init_color (HWND hwnd, ULONG idc)
{
  WinSendDlgItemMsg (hwnd, idc + 0, SPBM_SETLIMITS,
                     MPFROMLONG (255), MPFROMLONG (0));
  WinSendDlgItemMsg (hwnd, idc + 1, SPBM_SETLIMITS,
                     MPFROMLONG (255), MPFROMLONG (0));
  WinSendDlgItemMsg (hwnd, idc + 2, SPBM_SETLIMITS,
                     MPFROMLONG (255), MPFROMLONG (0));
}


static void enable_color (HWND hwnd, ULONG idc, ULONG enable)
{
  WinEnableWindow (WinWindowFromID (hwnd, idc + 0), enable);
  WinEnableWindow (WinWindowFromID (hwnd, idc + 1), enable);
  WinEnableWindow (WinWindowFromID (hwnd, idc + 2), enable);
}


static void enable_colors (HWND hwnd, ULONG stripes)
{
  enable_color (hwnd, IDC_COLOR2, stripes >= 2);
  enable_color (hwnd, IDC_COLOR3, stripes >= 3);
}


static ULONG get_stripes (HWND hwnd)
{
  ULONG stripes;

  WinSendDlgItemMsg (hwnd, IDC_STRIPES, SPBM_QUERYVALUE,
                     (MPARAM)&stripes, MPFROM2SHORT (0, SPBQ_UPDATEIFVALID));
  return (stripes);
}


static void set_stripes (HWND hwnd, ULONG stripes)
{
  WinSendDlgItemMsg (hwnd, IDC_STRIPES, SPBM_SETCURRENTVALUE,
                     MPFROMLONG (stripes), MPFROMLONG (0));
}


static ULONG get_orientation (HWND hwnd)
{
  MRESULT mr;

  mr = WinSendDlgItemMsg (hwnd, IDC_HORIZONTAL, BM_QUERYCHECK, NULL, NULL);
  return (SHORT1FROMMR (mr) ? HORIZONTAL : VERTICAL);
}


static void set_orientation (HWND hwnd, ULONG orientation)
{
  WinSendDlgItemMsg (hwnd, (orientation == VERTICAL
                            ? IDC_VERTICAL : IDC_HORIZONTAL), BM_SETCHECK,
                     MPFROMSHORT (TRUE), NULL);
}


typedef struct
{
  Flag   *somSelf;              /* Changing the settings of this object */
  ULONG  UndoColor[3];          /* Saved colors */
  ULONG  UndoStripes;           /* Saved number of stripes */
  ULONG  UndoOrientation;       /* Saved orientation */
  char   ignore;                /* Ignore SPBN_CHANGE while TRUE */
} FlagDlgDATA;


/* The dialog procedure. */

MRESULT EXPENTRY FlagDlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  FlagDlgDATA *pFlagDlgData;
  CHAR acBuffer[10];

  switch (msg)
    {
    case WM_INITDLG:
      {
        Flag *somSelf = ((WINDOWDATA *)mp2)->somSelf;
        ULONG i;

        pFlagDlgData = (FlagDlgDATA *)_wpAllocMem (somSelf,
                                            sizeof (*pFlagDlgData), NULL);
        if (pFlagDlgData == NULL)
          {
            WinSetWindowPtr (hwnd, QWL_USER, NULL);
            return ((MRESULT)FALSE);
          }
        memset (pFlagDlgData, 0, sizeof (*pFlagDlgData));
        pFlagDlgData->somSelf         = somSelf;
        pFlagDlgData->ignore          = TRUE;
        pFlagDlgData->UndoStripes     = _QueryStripes (somSelf);
        pFlagDlgData->UndoOrientation = _QueryOrientation (somSelf);
        for (i = 0; i < 3; ++i)
          pFlagDlgData->UndoColor[i] = _QueryColor (somSelf, i);

        WinSetWindowPtr (hwnd, QWL_USER, pFlagDlgData);
        init_color (hwnd, IDC_COLOR1);
        init_color (hwnd, IDC_COLOR2);
        init_color (hwnd, IDC_COLOR3);
        WinSendDlgItemMsg (hwnd, IDC_STRIPES, SPBM_SETLIMITS,
                           MPFROMLONG (3), MPFROMLONG (1));
        set_color (hwnd, IDC_COLOR1, _QueryColor (somSelf, 0));
        set_color (hwnd, IDC_COLOR2, _QueryColor (somSelf, 1));
        set_color (hwnd, IDC_COLOR3, _QueryColor (somSelf, 2));
        set_stripes (hwnd, _QueryStripes (somSelf));
        set_orientation (hwnd, _QueryOrientation (somSelf));
        enable_colors (hwnd, _QueryStripes (somSelf));
        pFlagDlgData->ignore = FALSE;
        return ((MRESULT)FALSE);
      }

    case WM_DESTROY:
      pFlagDlgData = WinQueryWindowPtr (hwnd, QWL_USER);
      if (pFlagDlgData != NULL)
        _wpFreeMem (pFlagDlgData->somSelf, (PBYTE)pFlagDlgData);
      break;

    case WM_COMMAND:
      pFlagDlgData = WinQueryWindowPtr (hwnd, QWL_USER);
      if (pFlagDlgData == NULL)
        break;
      switch (SHORT1FROMMP (mp1))
        {
        case IDC_UNDO:
          /* Note: If this changes the values, the window gets
             repainted by the control messages. */
          set_color (hwnd, IDC_COLOR1, pFlagDlgData->UndoColor[0]);
          set_color (hwnd, IDC_COLOR2, pFlagDlgData->UndoColor[1]);
          set_color (hwnd, IDC_COLOR3, pFlagDlgData->UndoColor[2]);
          set_stripes (hwnd, pFlagDlgData->UndoStripes);
          set_orientation (hwnd, pFlagDlgData->UndoOrientation);
          enable_colors (hwnd, _QueryStripes (pFlagDlgData->somSelf));
          break;

        case IDC_DEFAULT:
          /* Note: If this changes the values, the window gets
             repainted by the control messages. */
          set_color (hwnd, IDC_COLOR1, DEFAULT_COLOR1);
          set_color (hwnd, IDC_COLOR2, DEFAULT_COLOR2);
          set_color (hwnd, IDC_COLOR3, DEFAULT_COLOR3);
          set_stripes (hwnd, DEFAULT_STRIPES);
          set_orientation (hwnd, DEFAULT_ORIENTATION);
          enable_colors (hwnd, _QueryStripes (pFlagDlgData->somSelf));
          break;

        case IDC_HELP:
          break;
        }
      return ((MRESULT)TRUE);

    case WM_CONTROL:
      pFlagDlgData = WinQueryWindowPtr (hwnd, QWL_USER);
      if (pFlagDlgData == NULL)
        break;
      if (!pFlagDlgData->ignore)
        {
          switch (SHORT1FROMMP (mp1))
            {
            case IDC_HORIZONTAL:
            case IDC_VERTICAL:
              _SetOrientation (pFlagDlgData->somSelf, get_orientation (hwnd));
              _Repaint (pFlagDlgData->somSelf);
              break;

            case IDC_STRIPES:
              if (SHORT2FROMMP (mp1) == SPBN_CHANGE)
                {
                  _SetStripes (pFlagDlgData->somSelf, get_stripes (hwnd));
                  enable_colors (hwnd, _QueryStripes (pFlagDlgData->somSelf));
                  _Repaint (pFlagDlgData->somSelf);
                }
              break;

            case IDC_COLOR1_RED:
            case IDC_COLOR1_GREEN:
            case IDC_COLOR1_BLUE:
              if (SHORT2FROMMP (mp1) == SPBN_CHANGE)
                {
                  _SetColor (pFlagDlgData->somSelf, 0,
                             get_color (hwnd, IDC_COLOR1));
                  _Repaint (pFlagDlgData->somSelf);
                }
              break;

            case IDC_COLOR2_RED:
            case IDC_COLOR2_GREEN:
            case IDC_COLOR2_BLUE:
              if (SHORT2FROMMP (mp1) == SPBN_CHANGE)
                {
                  _SetColor (pFlagDlgData->somSelf, 1,
                             get_color (hwnd, IDC_COLOR2));
                  _Repaint (pFlagDlgData->somSelf);
                }
              break;

            case IDC_COLOR3_RED:
            case IDC_COLOR3_GREEN:
            case IDC_COLOR3_BLUE:
              if (SHORT2FROMMP (mp1) == SPBN_CHANGE)
                {
                  _SetColor (pFlagDlgData->somSelf, 2,
                             get_color (hwnd, IDC_COLOR3));
                  _Repaint (pFlagDlgData->somSelf);
                }
              break;
            }
        }
      return ((MRESULT)TRUE);
    }
  return (WinDefDlgProc (hwnd, msg, mp1, mp2));
}
