#ifndef __MY_MAIN_H__
#define __MY_MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	void main(void);
extern	void EventLoop(void);
extern	Boolean HandleSingleEvent(Boolean allowContextSwitching);
extern	short GetTheModifiers(void);
extern	void DispatchEvents(EventRecord theEvent, Boolean allowContextSwitching);
extern	void HandleMouseDown(EventRecord theEvent, Boolean allowContextSwitching);
extern	void ShutDownEnvironment(Boolean fullShutdown);

#ifdef __cplusplus
}
#endif

#endif
