/**********************************************************************

    Segment: MainBalloons.r

***********************************************************************/

/* THINK Rez resource format */
#include <Types.r>
#include <BalloonTypes.r>

resource 'hmnu' (400, "Apple", purgeable) {
/* header component */
  HelpMgrVersion,    /* version of Help Manager */
  hmDefaultOptions,  /* options */
  0,                 /* balloon definition function */
  0,                 /* variation code */
  
/* missing-items component */
  HMSkipItem {
    /* no missing items, so skip to menu-title component */
    },

/* BEGIN */ {

/* menu-title component */
  HMSkipItem {
    /* Apple menu has default help balloons */
    },
    
/* first menu-item component: About */
  HMStringItem {
    /* enabled */
    "Displays information about this application.",
    /* dimmed by application */
    "",
    /* checked */
    "",
    /* marked */
    "",
    },
    
/* END */ }
};

resource 'hmnu' (401, "File", purgeable) {
/* header component */
  HelpMgrVersion,    /* version of Help Manager */
  hmDefaultOptions,  /* options */
  0,                 /* balloon definition function */
  0,                 /* variation code */
  
/* missing-items component */
  HMSkipItem {
    /* no missing items, so skip to menu-title component */
    },

/* BEGIN */ {

/* menu-title component */
  HMStringItem {
    /* enabled */
    "File menu\n\nUse this menu to open documents or quit "
    "this application.",
    /* dimmed by application */
    "",
    /* dimmed by system (alerts or modal dialogs) */
    "File menu\n\nUse this menu to open documents or quit "
    "this application. Not available because there is a dialog box "
    "on the screen.",
    /* all items dimmed by system */
    "This item is not available because there is a dialog "
    "box on the screen.",
    },
    
/* first menu-item component: New */
  HMStringItem {
    /* enabled */
    "Creates new document window.",
    /* dimmed by application */
    "",
    /* checked */
    "",
    /* marked */
    "",
    },

/* second menu-item component: Close */
  HMStringItem {
    /* enabled */
    "Closes current document window.",
    /* dimmed by application */
    "Closes current document window. Not available now "
    "because the application does not currently have an "
    "open window.",
    /* checked */
    "",
    /* marked */
    "",
    },

/* third menu-item component: Quit */
  HMStringItem {
    /* enabled */
    "Quits this application.",
    /* dimmed by application */
    "",
    /* checked */
    "",
    /* marked */
    "",
    },
    
/* END */ }
};

resource 'hmnu' (402, "Edit", purgeable) {
/* header component */
  HelpMgrVersion,    /* version of Help Manager */
  hmDefaultOptions,  /* options */
  0,                 /* balloon definition function */
  0,                 /* variation code */
  
/* missing-items component */
  HMSkipItem {
    /* no missing items, so skip to menu-title component */
    },

/* BEGIN */ {

/* menu-title component */
  HMStringItem {
    /* enabled */
    "Edit menu\n\nUse this menu to manipulate text.",
    /* dimmed by application */
    "Edit menu\n\nUse this menu to manipulate text. "
    "Not available because there is nothing to edit.",
    /* dimmed by system (alerts or modal dialogs) */
    "Edit menu\n\nUse this menu to manipulate text. "
    "Not available because there is a dialog box on "
    "the screen.",
    /* all items dimmed by system */
    "This item is not available because there is a dialog "
    "box on the screen.",
    },
    
/* first menu-item component: Undo */
  HMStringItem {
    /* enabled */
    "Cancels your last edit.",
    /* dimmed by application */
    "Cancels your last edit. Not available because "
    "application doesn't support this function.",
    /* checked */
    "",
    /* marked */
    "",
    },
    
/* second menu-item component: Divider */
  HMSkipItem {
    /* no help balloons for divider lines */
    },
    
/* third menu-item component: Cut */
  HMStringItem {
    /* enabled */
    "Cuts selected text to the Clipboard.",
    /* dimmed by application */
    "Cuts selected text to the Clipboard. Not available now "
    "because no text is selected.",
    /* checked */
    "",
    /* marked */
    "",
    },

/* forth menu-item component: Copy */
  HMStringItem {
    /* enabled */
    "Copies selected text to the Clipboard.",
    /* dimmed by application */
    "Copies selected text to the Clipboard. Not available now "
    "because no text is selected.",
    /* checked */
    "",
    /* marked */
    "",
    },

/* fifth menu-item component: Paste */
  HMStringItem {
    /* enabled */
    "Inserts selected text from the Clipboard.",
    /* dimmed by application */
    "Inserts selected text from the Clipboard. Not available "
    "now because there is no text currently in the Clipboard.",
    /* checked */
    "",
    /* marked */
    "",
    },

/* sixth menu-item component: Clear */
  HMStringItem {
    /* enabled */
    "Deletes selected text.",
    /* dimmed by application */
    "Deletes selected text. Not available now "
    "because no text is selected.",
    /* checked */
    "",
    /* marked */
    "",
    },
    
/* END */ }
};

resource 'hmnu' (kHMHelpMenuID, "Help", purgeable)
{
    HelpMgrVersion, 0, 0, 0   /* header component             */
    HMSkipItem{               /* missing-items component      */
        /* no missing items, skip to first appended menu-item */
        /* component                                          */
        },
    {   /* first menu-item component */
        HMStringResItem{  /* use an 'STR#' for help messages  */
        800, 1,  /* 'STR#' res ID, index when item is enabled */
        800, 2,  /* 'STR#' res ID, index when item is dimmed  */
        800, 3,  /* 'STR#' res ID, index when item is checked */
        0, 0     /* item cannot be marked */
        },
    }
};

resource 'STR#' (800, "Help menu item strings")
{
    {
        /* array StringArray: six elements */
        /* [1] enabled help command */
        "Provides help for this application.";
        /* [2] dimmed help command */
        "Provides help for this application.";
        /* [3] checked help command */
        "Provides help for this application.";
    }
};

// End of File