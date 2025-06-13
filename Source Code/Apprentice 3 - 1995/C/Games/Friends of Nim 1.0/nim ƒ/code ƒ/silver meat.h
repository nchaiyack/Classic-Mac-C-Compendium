#ifndef __SILVER_MEAT_H__
#define __SILVER_MEAT_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	void InitSilver(void);
extern	void InitSilverOneGame(void);
extern	void SilverDrawWindow(WindowRef theWindow, short theDepth);
extern	void SilverClick(WindowRef theWindow, Point thePoint);
extern	void SilverIdle(WindowRef theWindow);

#ifdef __cplusplus
}
#endif

#endif
