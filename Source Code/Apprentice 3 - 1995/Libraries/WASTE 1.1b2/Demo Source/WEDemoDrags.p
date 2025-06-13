unit WEDemoDrags;

{ WASTE DEMO PROJECT: }
{ Drag Handlers }

{ Copyright © 1993-1995 Marco Piovanelli }
{ All Rights Reserved }

interface
	uses
		WEDemoIntf;

	function MyTrackingHandler (message: DragTrackingMessage;
									window: WindowPtr;
									refCon: Ptr;
									drag: DragReference): OSErr;
	function MyReceiveHandler (window: WindowPtr;
									refCon: Ptr;
									drag: DragReference): OSErr;

implementation

	function MyTrackingHandler (message: DragTrackingMessage;
									window: WindowPtr;
									refCon: Ptr;
									drag: DragReference): OSErr;
	begin
		if (window <> nil) & (WindowPeek(window)^.windowKind = userKind) then
			MyTrackingHandler := WETrackDrag(message, drag, DocumentPeek(window)^.hWE)
		else
			MyTrackingHandler := noErr;
	end;  { MyTrackingHandler }

	function MyReceiveHandler (window: WindowPtr;
									refCon: Ptr;
									drag: DragReference): OSErr;
	begin
		if (window <> nil) & (WindowPeek(window)^.windowKind = userKind) then
			MyReceiveHandler := WEReceiveDrag(drag, DocumentPeek(window)^.hWE)
		else
			MyReceiveHandler := noErr;
	end;  { MyReceiveHandler }

end.