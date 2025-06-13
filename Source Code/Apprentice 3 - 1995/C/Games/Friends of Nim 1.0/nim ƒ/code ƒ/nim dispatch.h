#ifndef __NIM_DISPATCH_H__
#define __NIM_DISPATCH_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	void InitGameDispatch(short game);
extern	void NewGameDispatch(short game, WindowRef theWindow);
extern	void NimDrawDispatch(short game, WindowRef theWindow, short theDepth);
extern	void NimClickDispatch(short game, WindowRef theWindow, Point thePoint);
extern	void NimIdleDispatch(short game, WindowRef theWindow);

#ifdef __cplusplus
}
#endif

#endif
