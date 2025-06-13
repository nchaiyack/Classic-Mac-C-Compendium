program WEDemoMain;

{ WASTE DEMO PROJECT: }
{ Main Program }

{ Copyright © 1993-1995 Marco Piovanelli }
{ All Rights Reserved }

	uses
		WEDemoEvents, WEDemoInit;

{ THINK Pascal compiler directive: turn off automatic initialization }

{$IFC NOT UNDEFINED THINK_PASCAL}
{$I-}
{$ENDC}

begin

	if (Initialize = noErr) then
		repeat
			ProcessEvent;
		until gExiting;

	Finalize;

end.