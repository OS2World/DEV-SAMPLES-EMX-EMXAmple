/* flag.csc -*- c -*- */


include <wpabs.sc>

class: Flag,
  external stem   = flag,
  local,
  external prefix = flag_,
  classprefix     = flagM_,
  major version   = 1,
  minor version   = 1;

parent: WPAbstract;

release order:
  InitWindowData, Repaint, AddFlagPage,
  QueryColor, SetColor,
  QueryStripes, SetStripes,
  QueryOrientation, SetOrientation,
  clsQueryModuleHandle;

passthru: C.ph;

typedef struct WINDOWDATA
{
  USHORT   cb;
  Flag     *somSelf;
  USEITEM  UseItem;
  VIEWITEM ViewItem;
  HWND     hwndClient;
} WINDOWDATA;

endpassthru;


passthru: C.ih;

#define INCL_WIN
#define INCL_GPI
#define INCL_DOS
#include <os2.h>

#define INCL_WPCLASS
#define INCL_WPFOLDER
#include <pmwp.h>

#define ID_ICON            1

#define ID_FRAME           1001
#define IDD_FLAG           1002

/* Don't change the order of the IDC_COLOR IDs! */

#define IDC_COLOR1         2001
#define IDC_COLOR1_RED     2001
#define IDC_COLOR1_GREEN   2002
#define IDC_COLOR1_BLUE    2003

#define IDC_COLOR2         2004
#define IDC_COLOR2_RED     2004
#define IDC_COLOR2_GREEN   2005
#define IDC_COLOR2_BLUE    2006

#define IDC_COLOR3         2007
#define IDC_COLOR3_RED     2007
#define IDC_COLOR3_GREEN   2008
#define IDC_COLOR3_BLUE    2009

#define IDC_STRIPES        2010
#define IDC_VERTICAL       2011
#define IDC_HORIZONTAL     2012

#define IDC_UNDO           2020
#define IDC_DEFAULT        2021
#define IDC_HELP           2022

#define IDM_OPENFLAG       (WPMENUID_USER+1)

#define OPEN_FLAG          IDM_OPENFLAG

#define IDKEY_COLOR1       1
#define IDKEY_COLOR2       2
#define IDKEY_COLOR3       3
#define IDKEY_STRIPES      4
#define IDKEY_ORIENTATION  5

#define VERTICAL           0
#define HORIZONTAL         1

/* Default settings. */

#define DEFAULT_COLOR1     RGB_BLUE
#define DEFAULT_COLOR2     RGB_WHITE
#define DEFAULT_COLOR3     RGB_RED
#define DEFAULT_STRIPES    3
#define DEFAULT_ORIENTATION HORIZONTAL


struct WINDOWDATA;

endpassthru;


passthru: C.h, after;
endpassthru;


/* Instance data */

data:
  ULONG color[3];
  ULONG stripes;
  ULONG orientation;
  struct WINDOWDATA *pWindowData;

# Methods

methods:

ULONG QueryColor (ULONG ulIndex);
ULONG QueryStripes ();
ULONG QueryOrientation ();
VOID SetColor (ULONG ulIndex, ULONG ulColor);
VOID SetStripes (ULONG ulStripes);
VOID SetOrientation (ULONG ulOrientation);
ULONG AddFlagPage (HWND hwndNotebook);
VOID Repaint ();
VOID InitWindowData (struct WINDOWDATA *pWindowData);


# Override methods

override wpAddSettingsPages;
override wpInitData;
override wpMenuItemSelected;
override wpModifyPopupMenu;
override wpOpen;
override wpRestoreState;
override wpSaveState;
override wpUnInitData;

# Class methods

HMODULE clsQueryModuleHandle(), class;

# Override class methods

override wpclsQueryDefaultView, class;
override wpclsQueryIconData, class;
override wpclsQueryStyle, class;
override wpclsQueryTitle, class;
