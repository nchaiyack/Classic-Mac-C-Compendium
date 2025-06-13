pascal OSErr myHandleOAPP(const AppleEvent *theAppleEvent, AppleEvent *reply,
					long handlerRefcon);
pascal OSErr myHandleODOC(const AppleEvent *theAppleEvent, AppleEvent *reply,
					long handlerRefcon);
pascal OSErr myHandlePDOC(const AppleEvent *theAppleEvent, AppleEvent *reply,
					long handlerRefcon);
pascal OSErr myHandleQUIT(const AppleEvent *theAppleEvent, AppleEvent *reply,
					long handlerRefcon);
pascal Boolean myIdleFunct(EventRecord *event, long *sleepTime, RgnHandle *mouseRg);
pascal OSErr myHandleIO(const AppleEvent *theAppleEvent, AppleEvent *reply,
					long handlerRefcon);
pascal OSErr myHandleGenericAlert(const AppleEvent *theAppleEvent, AppleEvent *reply,
					long handlerRefcon);
