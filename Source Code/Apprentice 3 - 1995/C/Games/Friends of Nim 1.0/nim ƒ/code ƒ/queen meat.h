#ifndef __QUEEN_MEAT_H__
#define __QUEEN_MEAT_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	void InitCorner(void);
extern	void InitCornerOneGame(WindowRef theWindow);
extern	void CornerDrawWindow(WindowRef theWindow, short theDepth);
extern	void CornerClick(WindowRef theWindow, Point thePoint);
extern	void CornerIdle(WindowRef theWindow);

#ifdef __cplusplus
}
#endif

#endif
