/**********************************************************************

    Segment: SampleDialogBalloons.r

***********************************************************************/

/* THINK Rez resource format */
#include <Types.r>
#include <BalloonTypes.r>

resource 'hdlg' (500, "Sample Dialog Balloons", purgeable){
  /* Header */
  HelpMgrVersion,
  0,                  /* start help with first item in DITL */
  hmDefaultOptions,   /* hmDefaultOptions or hmSaveBitsNoWindow */
  0,                  /* balloon definition */
  5,                  /* variation code or position code, reference
                         IM: More Macintosh Toolbox p3-10 */
  
  /* Missing Component */
  HMSkipItem {
  },
  
  /* Help */
  {
    /* [1] */
    HMStringREsItem { /* store help messages in STR# 500 */
    { 10, 10 },       /* default tip location is { 0, 0 } */
    { 0, 0, 0, 0 },   /* default alternate rectangle */
    500, 1,           /* OK button */
    0, 0,             /* never dimmed */
    0, 0,             /* never checked */
    0, 0              /* never marked */
    },
    
    /* [2] */
    HMStringREsItem {
    { 10, 10 },
    { 0, 0, 0, 0 },
    500, 2,           /* Cancel button */
    0, 0,
    0, 0,
    0, 0
    },
    
    /* [3] */
    HMSkipItem {      /* Name text */
    },

    /* [4] */
    HMSkipItem {      /* Phone text */
    },

    /* [5] */
    HMStringREsItem {
    { 10, 10 },
    { 0, 0, 0, 0 },
    500, 3,           /* Name field */
    0, 0,
    0, 0,
    0, 0
    },
    
    /* [6] */
    HMStringREsItem {
    { 10, 10 },
    { 0, 0, 0, 0 },
    500, 4,           /* Phone text */
    0, 0,
    0, 0,
    0, 0
    },

    /* [7] */
    HMStringREsItem {
    { 5, 5 },
    { 0, 0, 0, 0 },
    500, 5,           /* Male button */
    0, 0,
    500, 6,           /* checked */
    0, 0
    },

    /* [8] */
    HMStringREsItem {
    { 3, 3 },
    { 0, 0, 0, 0 },
    500, 7,           /* Female button */
    0, 0,
    500, 8,           /* checked */
    0, 0
    },

    /* [9] */
    HMSkipItem {      /* Phone text */
    },

    /* [10] */
    HMStringREsItem {
    { 10, 1 },
    { 0, 0, 0, 0 },
    500, 9,            /* Married box */
    0, 0,
    500, 10,           /* checked */
    0, 0
    },

    /* [11] */
    HMStringREsItem {
    { 10, 10 },
    { 0, 0, 0, 0 },
    0, 0,
    0, 0,
    500, 11,          /* Popup */
    500, 12           /* Popup */
    },
  }
};

resource 'STR#' (500, "Sample Dialog Help Strings") {
  {
  /* [1] */
  "To dismiss dialog and save changes, click this button.";
  /* [2] */
  "To dismiss dialog without saving changes, click this button.";
  /* [3] */
  "Enter persons name you wish to add to database.";
  /* [4] */
  "Enter persons phone number you wish to add "
  "to the database.";
  /* [5] */
  "Click this radio button to identify person as male.";
  /* [6] */
  "Identifies person as male.";
  /* [7] */
  "Click this radio button to identify person as female.";
  /* [8] */
  "Identifies person as female.";
  /* [9] */
  "Check this box if person is married.";
  /* [10] */
  "Identifies person as being married.";
  /* [11] */
  "Use pop-up menu to select a database to add person to."; 
  /* [12] */
  "Use pop-up menu to select a database to add person to."; 
  }
};

// End of File