#pragma once

enum depth
{
	k2Colors = 1,
	k16Colors = 4,
	k256Colors = 8,		//usually, you want 256 colors
						//primarily because it is widest supported
						//and simplifies color animation
	kThousandsColors = 24,
	kMillionsColors = 32,
	

//	However, the newer Macs can handle millions,
//	with enough memory, as Bungie can testify.

	kendsColors
};

Boolean GSSwitchDepth(depth inDepth);
void GSRestoreDepth();