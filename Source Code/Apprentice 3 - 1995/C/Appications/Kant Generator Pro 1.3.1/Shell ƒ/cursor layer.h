#ifndef __MY_CURSOR_LAYER_H__
#define __MY_CURSOR_LAYER_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	Boolean SetupTheAnimatedCursor(short acurID, short interval);
extern	void ShutdownTheAnimatedCursor(void);
extern	void AnimateTheCursor(void);

#ifdef __cplusplus
}
#endif

#endif
