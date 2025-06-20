#ifndef __TURN_MEAT_H__
#define __TURN_MEAT_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	void InitTurn(void);
extern	void InitTurnOneGame(void);
extern	void TurnDrawWindow(WindowRef theWindow, short theDepth);
extern	void TurnClick(WindowRef theWindow, Point thePoint);
extern	void TurnIdle(WindowRef theWindow);

#ifdef __cplusplus
}
#endif

#endif
