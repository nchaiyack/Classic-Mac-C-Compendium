void main(void);
void EventLoop(void);
Boolean HandleSingleEvent(Boolean allowContextSwitching);
void DispatchEvents(EventRecord theEvent, Boolean allowContextSwitching);
void HandleMouseDown(EventRecord theEvent, Boolean allowContextSwitching);
void ShutDownEnvironment(Boolean fullShutdown);
