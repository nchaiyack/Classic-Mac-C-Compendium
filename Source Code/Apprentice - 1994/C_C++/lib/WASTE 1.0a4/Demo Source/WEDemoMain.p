program WASTEDemo;

{ WASTE DEMO PROJECT: }
{ Main Program }

{ Copyright © 1993-1994 Merzwaren }
{ All Rights Reserved }

	uses
		DemoEvents, DemoInit;

{ THINK Pascal compiler directive: turn off automatic initialization }

{$IFC THINK_PASCAL}
{$I-}
{$ENDC}

begin

	if (Initialize = noErr) then
		repeat
			ProcessEvent;
		until gExiting;

	Finalize;

end.