#ifndef __NIM_END_GAME_H__
#define __NIM_END_GAME_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	void EndGame(WindowRef theWindow);
extern	Boolean IsComputersTurnQQ(void);
extern	void GetWinnerString(Str255 theStr);

#ifdef __cplusplus
}
#endif

#endif
