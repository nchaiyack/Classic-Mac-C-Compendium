#pragma once

void			Panic(void);
void			InitToolbox(void);
Boolean			InitGlobals(void);
void			SetUpMenus(void);
void 			InstallSplashScreen(void); 
void			ShowAbout(void);
void			DoMenu(long retVal);
void			DoMouseDown(EventRecord *curEvent);
void			DoKeyDown(EventRecord *curEvent);