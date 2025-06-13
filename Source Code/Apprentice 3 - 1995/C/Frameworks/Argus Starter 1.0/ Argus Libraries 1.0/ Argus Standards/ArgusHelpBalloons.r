/**********************************************************************

    Segment: ArgusAboutBalloons.r

***********************************************************************/

/* THINK Rez resource format */
#include <Types.r>
#include <BalloonTypes.r>

resource 'hdlg' (598, "Help Balloons", purgeable){
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
    598, 1,           /* OK button */
    0, 0,             /* never dimmed */
    0, 0,             /* never checked */
    0, 0              /* never marked */
    },
    
    /* [2] */
    HMStringREsItem {
    { 0, 0 },
    { 0, 0, 0, 0 },
    598, 2,           /* Print button */
    0, 0,
    0, 0,
    0, 0
    },

    /* [3] */
    HMStringREsItem {
    { 0, 0 },
    { 0, 0, 0, 0 },
    598, 3,           /* Save button */
    0, 0,
    0, 0,
    0, 0
    },

  }
};

resource 'STR#' (598, "Help Dialog Help Strings") {
  {
  /* [1] */
  "To dismiss dialog, click this button.";
  /* [2] */
  "Use to print above help information.";
  /* [3] */
  "Use to save above help information to a text file.";
  }
};

// End of File