enum regnamesstrs {
kREG_NAMES = 257,
kREG_EXTNAMES
};

enum regpopvalues {
kPOP_DEC = 1,
kPOP_HEX,
kPOP_OCT,
kPOP_BIN,
kPOP_TEXT = 6
};

enum {
kREG_HORZSEPLINE = 24,
kDIST_FROMLEFT = 2,
kDIST_FROMNAME = 1,
kDIST_FROMVERTSEP = 2,
kDISP_REGS = 13
};

#define REG_TOP		(kREG_HORZSEPLINE + 1)
#define REG_LEFT	(PRCT_L(gWPtr_Registers))
#define REG_BOTTOM	(PRCT_B(gWPtr_Registers))
#define REG_RIGHT	(PRCT_R(gWPtr_Registers))

/* these are semi-static variables used outside this file ONLY in the */
/* initialization routine */

extern ControlHandle	Ctrl_Base;
extern short	regLineHeight;
extern short	regDistFromTop;
extern short	regVertSepLine;
extern short	oldChoice;

/* end of semi-static variables */

void Update_Registers(WindowPtr w);
void Do_Registers(WindowPtr w, EventRecord *myEvent);
void ChangedRegister(short whichreg);
void ChangedAllRegisters(void);
void CloseRegisters(WindowPtr w);
void getDragRectRegs(WindowPtr w, RectPtr r);
