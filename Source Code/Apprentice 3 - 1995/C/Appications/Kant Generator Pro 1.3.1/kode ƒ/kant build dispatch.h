#ifndef __MY_BUILD_DISPATCH_H__
#define __MY_BUILD_DISPATCH_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	void DoNewDispatch(WindowRef theWindow, Boolean isRef);
extern	void DoEditDispatch(WindowRef theWindow);
extern	void DoDeleteDispatch(WindowRef theWindow);
extern	void DoMarkInterestingDispatch(WindowRef theWindow);
extern	void DoShowMessageDispatch(WindowRef theWindow);
extern	void DoShowToolbarDispatch(WindowRef theWindow);

#ifdef __cplusplus
}
#endif

#endif
