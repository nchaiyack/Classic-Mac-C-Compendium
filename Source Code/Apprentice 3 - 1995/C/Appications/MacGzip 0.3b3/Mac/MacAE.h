

/*****************************************************/
/* AE handlers */

pascal OSErr  MyHandleODoc(	AppleEvent *theAppleEvent,
							AppleEvent *reply,
							long handlerRefCon
							);
								
pascal OSErr  MyHandlePDoc(	AppleEvent *theAppleEvent,
							AppleEvent *reply,
							long handlerRefCon
							);
							
pascal OSErr  MyHandleOApp(	AppleEvent *theAppleEvent,
							AppleEvent *reply,
							long handlerRefCon
							);

pascal OSErr  MyHandleQuit(	AppleEvent *theAppleEvent,
							AppleEvent *reply,
							long handlerRefcon
							);	
													
OSErr MyGotRequiredParams( AppleEvent *theAppleEvent );



/*****************************************************/
/* Globals */

extern Boolean StartupFiles;
