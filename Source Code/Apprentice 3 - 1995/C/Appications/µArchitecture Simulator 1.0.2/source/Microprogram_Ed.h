enum {
kOPCODES = 128,
kRES_CNTL = 130,
kSTR_SEP = 132
};

enum clipmsgs {
kCLIPMSG_CUT = 0,
kCLIPMSG_COPY,
kCLIPMSG_PASTE,
kCLIPMSG_CLEAR
};

enum microedcntlitems {
kPOPUP_ABUS = 0,
kPOPUP_BBUS,
kPOPUP_CBUS,
kCHECK_MAR,
kCHECK_MBR,
kPOPUP_BUSREQ,
kPOPUP_SHIFTER,
kPOPUP_BRANCH,
kPOPUP_ALU,
kRADIO_ABUS,
kRADIO_MBR,
kCHECK_ACTMAP,
kNUM_CONTROLS
};

enum keytargets {
kKEY_BRTO = 0,
kKEY_COMMENT,	// attenzione a SimAsm.a !!!
kKEY_LIST,	// attenzione a SimAsm.a !!!
kKEY_INSTR,
kKEY_STRINGS,
kN_RECTS
};

enum microedlists {
kL_COMMENTS = 0,	// attenzione a SimAsm.a !!!
kL_INSTR
};

/* these are semi-static variables used outside this file ONLY in the */
/* initialization routine */

extern TEHandle TEs[2];
extern ListHandle	Lists[2];

extern RectPtr	keyrects[kN_RECTS];
extern ControlHandle	controls[kNUM_CONTROLS];
extern ControlHandle	RadioSelected;
extern short	keyDownDest;
extern short	theSelection[2];
extern short	maxLLine[2];

extern Boolean	arrowDrawn;
extern Boolean	draggedOnComments;

/* end of semi-static variables */

void EraseArrowRect(void);
void RefreshTE(short whichTE);
void SetControlsFromMir(union u_mir);
void Update_Microprogram_Ed(WindowPtr w);
void PrepareOpenMicroprogram(void);
void Activate_Microprogram_Ed(EventRecord *, WindowPtr w, Boolean activating);
void ChangedListSelection(Point, short, Boolean inCkLoop);
void Do_Microprogram_Ed(WindowPtr w, EventRecord *myEvent);
void Key_Microprogram_Ed(EventRecord *myEvent, Boolean ignore);
void Microprog_TextWasModified(void);
OSErr SendClipMsg(short msg);
void SelectLLine(short whichList, short newSelect);
void SetMir(unsigned long newmir);
void CloseMicroProg(WindowPtr w);
