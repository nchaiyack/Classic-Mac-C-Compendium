pascal OSErr  MyHandleODoc (AppleEvent *theAppleEvent, AppleEvent* reply, long
														handlerRefCon);
pascal OSErr  MyHandlePDoc (AppleEvent *theAppleEvent, AppleEvent *reply, long
														handlerRefCon);
pascal OSErr  MyHandleOApp (AppleEvent *theAppleEvent, AppleEvent *reply, long
														handlerRefCon);
pascal OSErr  MyHandleQuit (AppleEvent *theAppleEvent, AppleEvent *reply, long
														handlerRefCon);
OSErr MyGotRequiredParams (AppleEvent *theAppleEvent);
PROTO_UPP(MyHandleODoc, AEEventHandler);
PROTO_UPP(MyHandleOApp, AEEventHandler);
PROTO_UPP(MyHandlePDoc, AEEventHandler);
PROTO_UPP(MyHandleQuit, AEEventHandler);

