void DomyKeyEvent(EventRecord *);
void DoCloseWindow(WindowPtr w, short menuItem);
void DoUpdate(EventRecord *);
void DoActivate(EventRecord *);
void DecideActivation(EventRecord *, WindowPtr w, Point mouseGlob, Boolean active);
void DoOSEvent(EventRecord *);
void DoHighLevelEvent(EventRecord *evt);
void DoIdle(void);
void AdjustMenus(void);
