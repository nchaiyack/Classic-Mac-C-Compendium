enum {
kMaxCharsInOneDisasmLineFixed = 0x001C0000,
kMaxCharsInOneDisasmLine = 0x001C
};

extern	ControlHandle	disasmVScroll;
extern	short	disasmLineHeight, disasmFromTop, disasmCWidMax;

void DisasmHome(void);
void DisasmEnd(void);
void DisasmPgUp(void);
void DisasmPgDn(void);
void Activate_Disasm(EventRecord *, WindowPtr w, Boolean becomingActive);
void Grow_Disasm(WindowPtr w, EventRecord *event);
void Update_Disasm(WindowPtr w);
void Do_Disasm(WindowPtr w, EventRecord *event);
void DrawDisasm(WindowPtr w);
void SetupDisasmCtlMax(ControlHandle theControl);
void InvalDisasm(void);
void CloseDisasm(WindowPtr w);
void getDragRectDisasm(WindowPtr w, RectPtr r);
