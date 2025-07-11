enum {
kPART_NAMES = 256	/* STR# that contains part names */
};

enum objects {
kFIRST_PICT = 128,
kP_ALATCH = 128,
kP_BLATCH,
kP_MAR,
kP_MBR,
kP_AMUX,
kP_SHIFTER,
kP_INCR,
kP_MMUX,
kP_MPC,
kP_ALU,
kP_MSL,
kP_MIR,
kP_REGISTERS,
kP_CONTSTORE,
kP_MAP,
kP_MAR2MEM,
kP_MBRMEM,
kP_SH2MBR1,
kP_SH2MBR2,
kP_SH2REGS1,
kP_SH2REGS2,
kP_SH2REGS3,
kP_MBR2AMUX,
kP_AMUX2ALU,
kP_ALU2SH,
kP_REG2LTCH1,
kP_REG2LTCH2,
kP_ALTCH2AMUX,
kP_BLTCH2MAR1,
kP_BLTCH2MAR2,
kP_BLTCH2ALU,
kP_INC2MMUX1,
kP_INC2MMUX2,
kP_MMUX2MPC,
kP_MPC2INC,
kP_MPC2CST,
kP_CST2MIR,
kC_AMUX1,
kC_AMUX2,
kC_AMUX3,
kC_COND1,
kC_COND2,
kC_ALU1,
kC_ALU2,
kC_SHFT1,
kC_SHFT2,
kC_MBR1,
kC_MBR2,
kC_MBR3,
kC_READ1,
kC_READ2,
kC_READ3,
kC_WRITE1,
kC_WRITE2,
kC_WRITE3,
kC_MAR1,
kC_MAR2,
kC_MAR3,
kC_MAR4,
kC_DSC,
kC_ABC1,
kC_ABC2,
kC_ABC3,
kC_ABC4,
kC_ADDR1,
kC_ADDR2,
kC_ALU2MSL1,
kC_ALU2MSL2,
kC_MSL2MMUX1,
kC_MSL2MMUX2,
kP_MAPREGS,
kP_TEXT,
kLAST_PICT = kP_TEXT,
kMIRSubboxes = 14
};

/* these are semi-static variables used outside this file ONLY in the */
/* initialization routine */

extern GrafPtr	offScr;		/* offscreen GrafPort for animation */
extern PicHandle	images[kLAST_PICT - kFIRST_PICT + 1];	/* Handles to Pictures */
extern Rect	rval[kP_MIR - kP_ALATCH + kMIRSubboxes];	/* Rects of boxes containing values */
extern RgnHandle	mirUpdRgn;

/* end of semi-static variables */

void UpdateMir(un_mir m);
void ChangedBox(short obj);
void ChangedAllBoxes(void);
void ActivateObjs(const short *objs);
void DeactivateObjs(const short *objs);
void Update_Animation(WindowPtr w);
void Do_Animation(WindowPtr w, EventRecord *myEvent);
Boolean	TrackObject(RectPtr	r);
void CloseAnimation(WindowPtr w);
