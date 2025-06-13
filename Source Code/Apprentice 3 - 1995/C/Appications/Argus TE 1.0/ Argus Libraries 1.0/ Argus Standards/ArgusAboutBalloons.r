/**********************************************************************

    Segment: ArgusAboutBalloons.r

***********************************************************************/

/* THINK Rez resource format */
#include <Types.r>
#include <BalloonTypes.r>

resource 'hdlg' (599, "About Balloons", purgeable){
  /* Header */
  HelpMgrVersion,
  0,                  /* start help with first item in DITL */
  hmSaveBitsNoWindow, /* hmDefaultOptions or hmSaveBitsNoWindow */
  0,                  /* balloon definition */
  0,                  /* variation code or position code, reference
                         IM: More Macintosh Toolbox p3-10 */
  
  /* Missing Component */
  HMSkipItem {
  },
  
  /* Help */
  {
    /* [1] */
    HMStringREsItem { /* store help messages in STR# 500 */
    { 0, 0 },         /* default tip location is { 0, 0 } */
    { 0, 0, 0, 0 },   /* default alternate rectangle */
    599, 1,           /* OK button */
    0, 0,             /* never dimmed */
    0, 0,             /* never checked */
    0, 0              /* never marked */
    },
    
    /* [2] */
    HMStringREsItem {
    { 0, 0 },
    { 0, 0, 0, 0 },
    599, 2,           /* Help button */
    0, 0,
    0, 0,
    0, 0
    },
    
    /* [3] */
    HMSkipItem {      /* Picture text */
    },

    /* [4] */
    HMSkipItem {      /* Application name text */
    },

    /* [5] */
    HMSkipItem {      /* Application name text */
    },
  }
};

resource 'STR#' (599, "About Dialog Help Strings") {
  {
  /* [1] */
  "To dismiss dialog, click this button.";
  /* [2] */
  "To display help for this application, click this button.";
  }
};

// End of File