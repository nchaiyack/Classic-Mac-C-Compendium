#ifndef __PRIME_MEAT_H__
#define __PRIME_MEAT_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	void InitPrime(void);
extern	void InitPrimeOneGame(void);
extern	void PrimeDrawWindow(WindowRef theWindow, short theDepth);
extern	void PrimeClick(WindowRef theWindow, Point thePoint);
extern	void PrimeIdle(WindowRef theWindow);

#ifdef __cplusplus
}
#endif

#endif
