#ifndef __MY_TIMING_H__
#define __MY_TIMING_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	pascal void StartTiming(void);
extern	pascal void TimeCorrection(short tickCount);
extern	pascal void ScrollTheRect(Rect *r, short dh, short dv, RgnHandle unused);

#ifdef __cplusplus
}
#endif

#endif
