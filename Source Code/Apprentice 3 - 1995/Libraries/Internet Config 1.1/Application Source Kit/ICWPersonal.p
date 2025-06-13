unit ICWPersonal;

interface

	procedure DoWindoIdle (window: WindowPtr);
	procedure WindowDoKey (window: WindowPtr; modifiers: integer; ch: char; code: integer);
	procedure WindowActivateDeactivate (window: WindowPtr; activate: boolean);
	procedure WindowItemWhere (window: WindowPtr; er: EventRecord; item: integer);
	function WindowEarlyHandleEvent (window: WindowPtr; er: EventRecord): boolean;
	procedure WindowTab (window: WindowPtr; shift: boolean);
	function WindowEarlyHandleKey (window: WindowPtr; er: EventRecord): boolean;

implementation

end.
