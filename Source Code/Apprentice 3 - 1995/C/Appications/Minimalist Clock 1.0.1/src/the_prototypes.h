/* ----------------------------------------------------------------------
prototypes
---------------------------------------------------------------------- */

/* main.c */
extern void main();
extern void InitToolBox();
extern void InitMenuBar();
extern void MainLoop();
extern void Do();
extern void DoIdle();
extern void DoUpdate(WindowPtr window);
extern void DoMouseDown();
extern void DoMenu(long int menuChoice);

/* apple.c */
extern void DoMenuApple(int theItem);
extern void AppleAbout();
extern void AppleDA(int theItem);

/* file.c */
extern void DoMenuFile(int theItem);
extern void FileQuit();

/* edit.c */
extern void DoMenuEdit(int theItem);

/* clock.c */
extern void CreateClock();
extern void UpdateClock();
