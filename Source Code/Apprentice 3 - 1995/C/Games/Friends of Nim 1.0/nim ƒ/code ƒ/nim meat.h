#ifndef __NIM_MEAT_H__
#define __NIM_MEAT_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	void InitNim(void);
extern	void InitNimOneGame(void);
extern	void NimDrawWindow(WindowRef theWindow, short theDepth);
extern	void NimClick(WindowRef theWindow, Point thePoint);
extern	void NimIdle(WindowRef theWindow);

#ifdef __cplusplus
}
#endif

#endif
