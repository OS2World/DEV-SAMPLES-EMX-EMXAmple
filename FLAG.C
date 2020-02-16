/* flag.c (emx+gcc) */

/* This is a Workplace Shell sample program. */

/* There are several things which are not implemented:

   - Drag and drop (colors)
   - Icon in the Templates folder
   - Window title
   - Symmetry of the widths of the stripes
   - Help

   etc. */

#define USE_OS2_TOOLKIT_HEADERS

#define Flag_Class_Source
#define M_Flag_Class_Source

#include "flag.ih"
#include "flag.ph"

#include <stdlib.h>
#include <string.h>


/* Prototypes. */

static HWND FlagInit (Flag *somSelf);
MRESULT EXPENTRY FlagDlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY FlagWndProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);


/* Global data. */

static CHAR szFlagWindowClass[] = "FLAGSAMPLE";
static UCHAR szFlagClassTitle[CCHMAXPATH] = "Flag";
static HMODULE hmod = NULLHANDLE;


/* Instance methods. */

#undef SOM_CurrentClass
#define SOM_CurrentClass SOMInstance


/* Set pWindowData. */

SOM_Scope VOID SOMLINK
flag_InitWindowData (Flag *somSelf, WINDOWDATA *pWindowData)
{
  FlagData *somThis = FlagGetData (somSelf);
  _pWindowData = pWindowData;
}


/* Return the color of stripe ULINDEX (0 through 2). */

SOM_Scope ULONG SOMLINK
flag_QueryColor (Flag *somSelf, ULONG ulIndex)
{
  FlagData *somThis = FlagGetData (somSelf);
  return (_color[ulIndex]);
}


/* Set the color of stripe ULINDEX (0 through 2). */

SOM_Scope VOID SOMLINK
flag_SetColor (Flag *somSelf, ULONG ulIndex, ULONG ulColor)
{
  FlagData *somThis = FlagGetData (somSelf);
  _color[ulIndex] = ulColor;
}


/* Return the number of stripes. */

SOM_Scope ULONG SOMLINK
flag_QueryStripes (Flag *somSelf)
{
  FlagData *somThis = FlagGetData (somSelf);
  return (_stripes);
}


/* Set the number of stripes. */

SOM_Scope VOID SOMLINK
flag_SetStripes (Flag *somSelf, ULONG ulStripes)
{
  FlagData *somThis = FlagGetData (somSelf);
  _stripes = ulStripes;
}


/* Return the orientation of the stripes. */

SOM_Scope ULONG SOMLINK
flag_QueryOrientation (Flag *somSelf)
{
  FlagData *somThis = FlagGetData (somSelf);
  return (_orientation);
}


/* Set the orientation of the stripes. */

SOM_Scope VOID SOMLINK
flag_SetOrientation (Flag *somSelf, ULONG ulOrientation)
{
  FlagData *somThis = FlagGetData (somSelf);
  _orientation = ulOrientation;
}


/* Add the "Flag" page to the settings notebook. */

SOM_Scope ULONG SOMLINK
flag_AddFlagPage (Flag *somSelf, HWND hwndNotebook)
{
  PAGEINFO pageinfo;
  static WINDOWDATA wd;
  WINDOWDATA *pwd;
  FlagData *somThis = FlagGetData (somSelf);

  pwd = _pWindowData;
  if (pwd == NULL)
    {
      /* Pass a dummy WINDOWDATA structure if the Flag view is not
         open.  FlagDlgProc takes somSelf from this structure. */

      memset (&wd, 0, sizeof (wd));
      wd.cb = sizeof (wd);
      wd.somSelf = somSelf;
      pwd = &wd;
      if (hmod == NULLHANDLE)
        return (TRUE);
    }
  memset (&pageinfo, 0, sizeof (pageinfo));
  pageinfo.cb                 = sizeof (pageinfo);
  pageinfo.hwndPage           = NULLHANDLE;
  pageinfo.usPageStyleFlags   = BKA_MAJOR;
  pageinfo.usPageInsertFlags  = BKA_FIRST;
  pageinfo.pfnwp              = FlagDlgProc;
  pageinfo.resid              = hmod;
  pageinfo.dlgid              = IDD_FLAG;
  pageinfo.pszName            = "Flag";
  pageinfo.pCreateParams      = pwd;
  pageinfo.idDefaultHelpPanel = 0;
  pageinfo.pszHelpLibraryName = NULL;
  return (_wpInsertSettingsPage (somSelf, hwndNotebook, &pageinfo));
}


/* Repaint the window. */

SOM_Scope VOID SOMLINK
flag_Repaint (Flag *somSelf)
{
  FlagData *somThis = FlagGetData (somSelf);

  if (_pWindowData != NULL && _pWindowData->hwndClient != NULLHANDLE)
    WinInvalidateRect (_pWindowData->hwndClient, NULL, FALSE);
}


/* Initialize instance data. */

SOM_Scope void SOMLINK
flag_wpInitData (Flag *somSelf)
{
  FlagData *somThis = FlagGetData (somSelf);

  memset (somThis, 0, sizeof (*somThis));
  _color[0]    = DEFAULT_COLOR1;
  _color[1]    = DEFAULT_COLOR2;
  _color[2]    = DEFAULT_COLOR3;
  _stripes     = DEFAULT_STRIPES;
  _orientation = DEFAULT_ORIENTATION;
  parent_wpInitData (somSelf);
}


/* Free instance data. */

SOM_Scope void SOMLINK
flag_wpUnInitData (Flag *somSelf)
{
  FlagData *somThis = FlagGetData (somSelf);

  parent_wpUnInitData (somSelf);
}


/* Save the state of the object. */

SOM_Scope BOOL SOMLINK
flag_wpSaveState (Flag *somSelf)
{
  FlagData *somThis = FlagGetData (somSelf);

  _wpSaveLong (somSelf, szFlagClassTitle, IDKEY_COLOR1, _color[0]);
  _wpSaveLong (somSelf, szFlagClassTitle, IDKEY_COLOR2, _color[1]);
  _wpSaveLong (somSelf, szFlagClassTitle, IDKEY_COLOR3, _color[2]);
  _wpSaveLong (somSelf, szFlagClassTitle, IDKEY_STRIPES, _stripes);
  _wpSaveLong (somSelf, szFlagClassTitle, IDKEY_ORIENTATION, _orientation);
  return (parent_wpSaveState (somSelf));
}


/* Restore the state of the object. */

SOM_Scope BOOL SOMLINK
flag_wpRestoreState (Flag *somSelf, ULONG ulReserved)
{
  FlagData *somThis = FlagGetData (somSelf);

  _wpRestoreLong (somSelf, szFlagClassTitle, IDKEY_COLOR1, &_color[0]);
  _wpRestoreLong (somSelf, szFlagClassTitle, IDKEY_COLOR2, &_color[1]);
  _wpRestoreLong (somSelf, szFlagClassTitle, IDKEY_COLOR3, &_color[2]);
  _wpRestoreLong (somSelf, szFlagClassTitle, IDKEY_STRIPES, &_stripes);
  _wpRestoreLong (somSelf, szFlagClassTitle, IDKEY_ORIENTATION, &_orientation);
  return (parent_wpRestoreState (somSelf, ulReserved));
}


/* Add pages to the settings notebook. */

SOM_Scope BOOL SOMLINK
flag_wpAddSettingsPages (Flag *somSelf, HWND hwndNotebook)
{
  FlagData *somThis = FlagGetData (somSelf);

  return (parent_wpAddSettingsPages (somSelf, hwndNotebook)
          && _AddFlagPage (somSelf, hwndNotebook));
}


/* Open a view to the object. */

SOM_Scope HWND SOMLINK
flag_wpOpen (Flag *somSelf, HWND hwndCnr, ULONG ulView, ULONG param)
{
 FlagData *somThis = FlagGetData (somSelf);

 switch (ulView)
   {
   case OPEN_FLAG:
     if (!_wpSwitchTo (somSelf, ulView))
       return (FlagInit (somSelf));

   default:
     return (parent_wpOpen (somSelf, hwndCnr, ulView, param));
   }
}


/* Add menu item. */

SOM_Scope BOOL SOMLINK
flag_wpModifyPopupMenu (Flag *somSelf, HWND hwndMenu, HWND hwndCnr,
                        ULONG iPosition)
{
  FlagData *somThis = FlagGetData (somSelf);

  _wpInsertPopupMenuItems (somSelf, hwndMenu, 0, hmod, IDM_OPENFLAG,
                           WPMENUID_OPEN);
  return (parent_wpModifyPopupMenu (somSelf, hwndMenu, hwndCnr, iPosition));
}


/* Menu item selected. */

SOM_Scope BOOL SOMLINK
flag_wpMenuItemSelected (Flag *somSelf, HWND hwndFrame, ULONG MenuId)
{
  FlagData *somThis = FlagGetData (somSelf);

  switch (MenuId)
    {
    case IDM_OPENFLAG:
      _wpViewObject (somSelf, NULLHANDLE, OPEN_FLAG, 0);
      break;

    default:
      return (parent_wpMenuItemSelected (somSelf, hwndFrame, MenuId));
      break;
    }
  return (TRUE);
}


/* Class methods. */

#undef SOM_CurrentClass
#define SOM_CurrentClass SOMMeta


/* Return the module handle. */

SOM_Scope HMODULE SOMLINK
flagM_clsQueryModuleHandle (M_Flag *somSelf)
{
 if (hmod == NULLHANDLE)
   {
     zString szPathName;

     szPathName = _somLocateClassFile (SOMClassMgrObject,
                                       SOM_IdFromString ("Flag"),
                                       Flag_MajorVersion, Flag_MinorVersion);
     DosQueryModuleHandle (szPathName, &hmod);
   }
 return (hmod);
}


/* Return the class style: Inhibit creation of a shadow. */

SOM_Scope ULONG SOMLINK
flagM_wpclsQueryStyle (M_Flag *somSelf)
{
  return (parent_wpclsQueryStyle (somSelf) | CLSSTYLE_NEVERLINK);
}


/* Return the default title for the instances. */

SOM_Scope PSZ SOMLINK
flagM_wpclsQueryTitle (M_Flag *somSelf)
{
  return (szFlagClassTitle);
}


/* Build the class default icon. */

SOM_Scope ULONG SOMLINK
flagM_wpclsQueryIconData (M_Flag *somSelf, PICONINFO pIconInfo)
{
  if (pIconInfo != NULL)
    {
      pIconInfo->fFormat = ICON_RESOURCE;
      pIconInfo->hmod    = _clsQueryModuleHandle (somSelf);
      pIconInfo->resid   = ID_ICON;
    }
  return (sizeof (ICONINFO));
}


/* Return the default view of this class. */

SOM_Scope ULONG SOMLINK
flagM_wpclsQueryDefaultView (M_Flag *somSelf)
{
  return (OPEN_FLAG);
}


/* Ordinary code. */

#undef SOM_CurrentClass


/* Initialize a view. */

HWND FlagInit (Flag *somSelf)
{
  HAB hab;
  HWND hwndFrame;
  HWND hwndClient;
  WINDOWDATA *pWindowData;
  FRAMECDATA flFrameCtlData;

  hab = WinQueryAnchorBlock (HWND_DESKTOP);
  WinRegisterClass (hab, szFlagWindowClass, (PFNWP)FlagWndProc ,
                    CS_SIZEREDRAW | CS_SYNCPAINT, sizeof (pWindowData));

  pWindowData = (WINDOWDATA *)_wpAllocMem (somSelf, sizeof (*pWindowData),
                                           NULL);
  if (pWindowData == NULL)
    return (NULLHANDLE);

  memset (pWindowData, 0, sizeof (*pWindowData));
  pWindowData->cb = sizeof (*pWindowData);
  pWindowData->somSelf = somSelf;

  _InitWindowData (somSelf, pWindowData);

  flFrameCtlData.cb            = sizeof (flFrameCtlData);
  flFrameCtlData.flCreateFlags = (FCF_SIZEBORDER | FCF_TITLEBAR
                                  | FCF_SYSMENU  | FCF_MINMAX);
  flFrameCtlData.hmodResources = hmod;
  flFrameCtlData.idResources   = ID_ICON;

  hwndFrame = WinCreateWindow (HWND_DESKTOP, WC_FRAME,
                               _wpQueryTitle (somSelf), 0,
                               0, 0, 0, 0,
                               NULLHANDLE, HWND_TOP, ID_FRAME,
                               (PVOID)&flFrameCtlData, NULL);
  if (hwndFrame == NULLHANDLE)
    return (NULLHANDLE);

  hwndClient = WinCreateWindow (hwndFrame, szFlagWindowClass,
                                NULL, 0,
                                0, 0, 0, 0,
                                hwndFrame, HWND_TOP, FID_CLIENT,
                                pWindowData, NULL);
  if (hwndClient == NULLHANDLE)
    {
      WinDestroyWindow (hwndFrame);
      return (NULLHANDLE);
    }
  pWindowData->hwndClient = hwndClient;

  WinSendMsg (hwndFrame, WM_SETICON, MPFROMP (_wpQueryIcon (somSelf)), NULL);

  if (!WinRestoreWindowPos (szFlagClassTitle, _wpQueryTitle (somSelf),
                            hwndFrame))
    {
      SWP swp;

      WinQueryTaskSizePos (hab, 0, &swp);
      swp.fl = SWP_SIZE|SWP_MOVE|SWP_RESTORE|SWP_ZORDER;
      WinSetWindowPos (hwndFrame, HWND_TOP, swp.x, swp.y, swp.cx,
                       swp.cy, swp.fl);
    }
  WinShowWindow (hwndFrame, TRUE);
  return (hwndFrame);
}


/* The window procedure. */

MRESULT EXPENTRY FlagWndProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  ULONG MenuId;
  WINDOWDATA *pWindowData;
  HWND hwndFrame;
  CHAR acBuffer[10];

  hwndFrame = WinQueryWindow (hwnd, QW_PARENT);

  switch (msg)
    {
    case WM_CREATE:
      pWindowData = (WINDOWDATA *)mp1;
      pWindowData->UseItem.type    = USAGE_OPENVIEW;
      pWindowData->ViewItem.view   = OPEN_FLAG;
      pWindowData->ViewItem.handle = hwndFrame;

      WinSetWindowPtr (hwnd, QWL_USER, pWindowData);
      _wpAddToObjUseList (pWindowData->somSelf, &pWindowData->UseItem);
      _wpRegisterView (pWindowData->somSelf, hwndFrame,
                       _wpQueryTitle (pWindowData->somSelf));
      WinSetFocus (HWND_DESKTOP, hwndFrame);
      break;

    case WM_PAINT:
      {
        HPS hps;
        RECTL rectl;
        ULONG i, stripes, dx, dy;

        pWindowData = WinQueryWindowPtr (hwnd, QWL_USER);
        if (pWindowData == NULL)
          break;
        stripes = _QueryStripes (pWindowData->somSelf);
        WinQueryWindowRect (hwnd, &rectl);
        if (_QueryOrientation (pWindowData->somSelf) == VERTICAL)
          {
            dx = (rectl.xRight + stripes - 1) / stripes; dy = 0;
            rectl.xRight = dx;
          }
        else
          {
            dy = (rectl.yTop + stripes - 1) / stripes; dx = 0;
            rectl.yBottom = rectl.yTop - dy;
          }
        hps = WinBeginPaint (hwnd, NULLHANDLE, NULL);
        GpiCreateLogColorTable (hps, LCOL_PURECOLOR, LCOLF_RGB, 0, 0, NULL);
        for (i = 0; i < stripes; ++i)
          {
            WinFillRect (hps, &rectl, _QueryColor (pWindowData->somSelf, i));
            rectl.xLeft += dx; rectl.xRight += dx;
            rectl.yBottom -= dy; rectl.yTop -= dy;
          }
        WinEndPaint (hps);
      }
      break;

    case WM_CLOSE:
      {
        HAB hab;

        pWindowData = (WINDOWDATA *)WinQueryWindowPtr (hwnd, QWL_USER);
        if (pWindowData == NULL)
          break;
        hab = WinQueryAnchorBlock (HWND_DESKTOP);
        WinStoreWindowPos (szFlagClassTitle,
                           _wpQueryTitle (pWindowData->somSelf), hwndFrame);
        _wpDeleteFromObjUseList (pWindowData->somSelf, &pWindowData->UseItem);
        _InitWindowData (pWindowData->somSelf, NULL);
        _wpFreeMem (pWindowData->somSelf, (PBYTE)pWindowData);
        WinPostMsg (hwnd, WM_QUIT, 0, 0);
        WinDestroyWindow (hwndFrame);
      }
      break;
    }
  return (WinDefWindowProc (hwnd, msg, mp1, mp2));
}
