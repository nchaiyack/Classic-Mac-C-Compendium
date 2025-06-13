/*
Copyright © 1993,1994 by Fabrizio Oddone
••• ••• ••• ••• ••• ••• ••• ••• ••• •••
This source code is distributed as freeware: you can copy, exchange, modify this
code as you wish. You may include this code in any kind of application: freeware,
shareware, or commercial, provided that full credits are given.
You may not sell or distribute this code for profit.
*/

#include "BalloonTypes.r"

resource 'hfdr' (-5696, purgeable) {
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,
	{
		HMTEResItem {
			1000
		}
	}
};

resource 'hmnu' (1, purgeable) {
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,

		HMSkipItem { },			/* item -1:Missing menu items */
	{

		HMSkipItem { },			/* item 0:Menu Title */
		HMStringResItem {		/* item 1:Open… */
			1000,1,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 2:Save As… */
			1000,2,
			0,0,
			0,0,
			0,0
		}
	}
};

resource 'hmnu' (2, purgeable) {
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,

		HMSkipItem { },			/* item -1:Missing menu items */
	{

		HMSkipItem { },			/* item 0:Menu Title */
		HMStringResItem {		/* item 1:Open… */
			1000,3,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 2:Save As… */
			1000,4,
			0,0,
			0,0,
			0,0
		}
	}
};

resource 'hmnu' (3, purgeable) {
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,

		HMStringResItem {		/* item -1:Missing menu items */
			1000,5,
			0,0,
			0,0,
			0,0
		},
	{

	}
};

resource 'hmnu' (4, purgeable) {
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,

		HMStringResItem {		/* item -1:Missing menu items */
			1000,6,
			0,0,
			0,0,
			0,0
		},
	{

	}
};

resource 'hmnu' (256, purgeable) {
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,

		HMSkipItem { },			/* item -1:Missing menu items */
	{

		HMSkipItem { },			/* item 0:Menu Title */
		HMStringResItem {		/* item 1:Decimal */
			1001,1,
			0,0,
			1001,2,
			0,0
		},
		HMStringResItem {		/* item 2:Hexadecimal */
			1001,3,
			0,0,
			1001,4,
			0,0
		},
		HMStringResItem {		/* item 3:Octal */
			1001,5,
			0,0,
			1001,6,
			0,0
		},
		HMStringResItem {		/* item 4:Binary */
			1001,7,
			0,0,
			1001,8,
			0,0
		},
			HMSkipItem { },			/* sep line */
		HMStringResItem {		/* item 6:as text */
			1001,9,
			0,0,
			1001,10,
			0,0
		}
	}
};

resource 'hmnu' (257, purgeable) {
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,

		HMSkipItem { },			/* item -1:Missing menu items */
	{

		HMSkipItem { },			/* item 0:Menu Title */
		HMStringResItem {		/* item 1:Dec */
			1001,11,
			0,0,
			1001,12,
			0,0
		},
		HMStringResItem {		/* item 2:Hex */
			1001,13,
			0,0,
			1001,14,
			0,0
		},
		HMStringResItem {		/* item 3:Oct */
			1001,15,
			0,0,
			1001,16,
			0,0
		},
		HMStringResItem {		/* item 4:Bin */
			1001,17,
			0,0,
			1001,18,
			0,0
		},
			HMSkipItem { },			/* sep line */
		HMStringResItem {		/* item 6:as text */
			1001,19,
			0,0,
			1001,20,
			0,0
		}
	}
};

resource 'hmnu' (258, purgeable) {	/* Shifter: */
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,

		HMSkipItem { },			/* item -1:Missing menu items */
	{

		HMSkipItem { },			/* item 0:Menu Title */
		HMStringResItem {		/* item 1:Inactive */
			1001,21,
			0,0,
			1001,22,
			0,0
		},
		HMStringResItem {		/* item 2:Left */
			1001,23,
			0,0,
			1001,24,
			0,0
		},
		HMStringResItem {		/* item 3:Right Logical */
			1001,25,
			0,0,
			1001,26,
			0,0
		},
		HMStringResItem {		/* item 4:Right Arithmetic */
			1001,27,
			0,0,
			1001,28,
			0,0
		}
	}
};

resource 'hmnu' (259, purgeable) {	/* Branch: */
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,

		HMSkipItem { },			/* item -1:Missing menu items */
	{

		HMSkipItem { },			/* item 0:Menu Title */
		HMStringResItem {		/* item 1:never */
			1001,29,
			0,0,
			1001,30,
			0,0
		},
		HMStringResItem {		/* item 2:if n */
			1001,31,
			0,0,
			1001,32,
			0,0
		},
		HMStringResItem {		/* item 3:if z */
			1001,33,
			0,0,
			1001,34,
			0,0
		},
		HMStringResItem {		/* item 4:always */
			1001,35,
			0,0,
			1001,36,
			0,0
		}
	}
};

resource 'hmnu' (260, purgeable) {	/* ALU: */
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,

		HMSkipItem { },			/* item -1:Missing menu items */
	{

		HMSkipItem { },			/* item 0:Menu Title */
		HMStringResItem {		/* item 1:Inactive */
			1001,37,
			0,0,
			1001,38,
			0,0
		},
		HMStringResItem {		/* item 2:+ */
			1001,39,
			0,0,
			1001,40,
			0,0
		},
		HMStringResItem {		/* item 3:& */
			1001,41,
			0,0,
			1001,42,
			0,0
		},
		HMStringResItem {		/* item 4:~ */
			1001,43,
			0,0,
			1001,44,
			0,0
		}
	}
};

resource 'hmnu' (261, purgeable) {	/* A Bus: */
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,

		HMSkipItem { },			/* item -1:Missing menu items */
	{

		HMSkipItem { },			/* item 0:Menu Title */
		HMStringResItem {		/* item 1:PC */
			1002,1,
			0,0,
			1002,2,
			0,0
		},
		HMStringResItem {		/* item 2:AC */
			1002,3,
			0,0,
			1002,4,
			0,0
		},
		HMStringResItem {		/* item 3:SP */
			1002,5,
			0,0,
			1002,6,
			0,0
		},
		HMStringResItem {		/* item 4:BP */
			1002,7,
			0,0,
			1002,8,
			0,0
		},
		HMStringResItem {		/* item 5:IR */
			1002,9,
			0,0,
			1002,10,
			0,0
		},
		HMStringResItem {		/* item 6:A */
			1002,11,
			0,0,
			1002,12,
			0,0
		},
		HMStringResItem {		/* item 7:B */
			1002,13,
			0,0,
			1002,14,
			0,0
		},
		HMStringResItem {		/* item 8:C */
			1002,15,
			0,0,
			1002,16,
			0,0
		},
		HMStringResItem {		/* item 9:D */
			1002,17,
			0,0,
			1002,18,
			0,0
		},
		HMStringResItem {		/* item 10:E */
			1002,19,
			0,0,
			1002,20,
			0,0
		},
		HMStringResItem {		/* item 11:X12 */
			1002,21,
			0,0,
			1002,22,
			0,0
		},
		HMStringResItem {		/* item 12:X11 */
			1002,23,
			0,0,
			1002,24,
			0,0
		},
		HMStringResItem {		/* item 13:Lo8 */
			1002,25,
			0,0,
			1002,26,
			0,0
		},
		HMStringResItem {		/* item 14:Zero */
			1002,27,
			0,0,
			1002,28,
			0,0
		},
		HMStringResItem {		/* item 15:One */
			1002,29,
			0,0,
			1002,30,
			0,0
		},
		HMStringResItem {		/* item 16:MinusOne */
			1002,31,
			0,0,
			1002,32,
			0,0
		}
	}
};

resource 'hmnu' (262, purgeable) {	/* C Bus: */
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,

		HMSkipItem { },			/* item -1:Missing menu items */
	{

		HMSkipItem { },			/* item 0:Menu Title */
		HMStringResItem {		/* item 1:Disabled */
			1003,1,
			0,0,
			1003,2,
			0,0
		},
		HMStringResItem {		/* item 2:PC */
			1003,3,
			0,0,
			1003,4,
			0,0
		},
		HMStringResItem {		/* item 3:AC */
			1003,5,
			0,0,
			1003,6,
			0,0
		},
		HMStringResItem {		/* item 4:SP */
			1003,7,
			0,0,
			1003,8,
			0,0
		},
		HMStringResItem {		/* item 5:BP */
			1003,9,
			0,0,
			1003,10,
			0,0
		},
		HMStringResItem {		/* item 6:IR */
			1003,11,
			0,0,
			1003,12,
			0,0
		},
		HMStringResItem {		/* item 7:A */
			1003,13,
			0,0,
			1003,14,
			0,0
		},
		HMStringResItem {		/* item 8:B */
			1003,15,
			0,0,
			1003,16,
			0,0
		},
		HMStringResItem {		/* item 9:C */
			1003,17,
			0,0,
			1003,18,
			0,0
		},
		HMStringResItem {		/* item 10:D */
			1003,19,
			0,0,
			1003,20,
			0,0
		},
		HMStringResItem {		/* item 11:E */
			1003,21,
			0,0,
			1003,22,
			0,0
		}
	}
};

resource 'hmnu' (263, purgeable) {	/* Bus request: */
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,

		HMSkipItem { },			/* item -1:Missing menu items */
	{

		HMSkipItem { },			/* item 0:Menu Title */
		HMStringResItem {		/* item 1:No */
			1003,23,
			0,0,
			1003,24,
			0,0
		},
		HMStringResItem {		/* item 2:Read */
			1003,25,
			0,0,
			1003,26,
			0,0
		},
		HMStringResItem {		/* item 3:Write */
			1003,27,
			0,0,
			1003,28,
			0,0
		}
	}
};

resource 'hmnu' (128, purgeable) {
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,

		HMSkipItem { },			/* item -1:Missing menu items */
	{

		HMSkipItem { },			/* item 0:Menu Title */
		HMStringResItem {		/* item 1:About µSimulator… */
			1004,3,
			0,0,
			0,0,
			0,0
		}
	}
};

resource 'hmnu' (129, purgeable) {	/* File */
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,

		HMSkipItem { },			/* item -1:Missing menu items */
	{

		HMStringResItem {		/* Menu Title */
			1004,4,
			0,0,
			1004,1,
			1004,2
		},
		HMStringResItem {		/* item 1:New */
			1004,5,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 2:Open… */
			1004,6,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 4:Close Control Store */
			1004,7,
			1004,8,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 5:Save Control Store */
			1004,9,
			1004,10,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 6:Save Control Store As… */
			1004,11,
			1004,12,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 6:Save Control Store As Stationery… */
			1004,19,
			1004,12,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 7:Revert to Saved */
			1004,13,
			1004,14,
			0,0,
			0,0
		},
					HMSkipItem { },
					HMSkipItem { },
					HMSkipItem { },	/* item 10:Registers */
					HMSkipItem { },
		HMStringResItem {		/* item 12:Page Setup… */
			0,0,
			1004,15,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 13:Print… */
			0,0,
			1004,16,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 15:Preferences… */
			1004,17,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 17:Quit */
			1004,18,
			0,0,
			0,0,
			0,0
		}
	}
};

resource 'hmnu' (130, purgeable) {	/* Edit */
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,

		HMSkipItem { },			/* item -1:Missing menu items */
	{

		HMStringResItem {		/* Menu Title */
			1005,1,
			0,0,
			1004,1,
			1004,2
		},
		HMStringResItem {		/* item 1:Undo */
			1005,2,
			1005,3,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 3:Cut */
			1005,4,
			1005,5,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 4:Copy */
			1005,6,
			1005,7,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 5:Paste */
			1005,8,
			1005,9,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 6:Clear */
			1005,10,
			1005,11,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 7:Select All */
			1005,12,
			1005,13,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 9:Insert Line */
			1005,14,
			1005,15,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 10:Delete Line */
			1005,16,
			1005,17,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 8:Copy e-mail */
			1200,1,
			1000,4,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 5:Copy WWW */
			1200,2,
			1000,4,
			0,0,
			0,0
		}
	}
};

resource 'hmnu' (131, purgeable) {	/* Windows */
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,

		HMSkipItem { },			/* item -1:Missing menu items */
	{

		HMStringResItem {		/* Menu Title */
			1006,1,
			0,0,
			1004,1,
			1004,2
		},
		HMStringResItem {		/* item 1:Animation */
			1006,2,
			0,0,
			1006,3,
			0,0
		},
		HMStringResItem {		/* item 2:Registers */
			1006,4,
			0,0,
			1006,5,
			0,0
		},
		HMStringResItem {		/* item 3:Input/Output */
			1006,6,
			0,0,
			1006,7,
			0,0
		},
		HMStringResItem {		/* item 4:Microprogram */
			1006,8,
			1006,9,
			1006,10,
			0,0
		},
		HMStringResItem {		/* item 5:Dump */
			1006,11,
			0,0,
			1006,12,
			0,0
		},
		HMStringResItem {		/* item 6:Disassembler */
			1006,13,
			0,0,
			1006,14,
			0,0
		}
	}
};

resource 'hmnu' (132, purgeable) {	/* Control */
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,

		HMSkipItem { },			/* item -1:Missing menu items */
	{

		HMStringResItem {		/* Menu Title */
			1007,1,
			0,0,
			1004,1,
			1004,2
		},
		HMStringResItem {		/* item 1:Go */
			1007,2,
			1007,3,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 2:Stop */
			1007,4,
			1007,5,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 4:Step sub */
			1007,7,
			1007,6,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 5:Step micro */
			1007,8,
			1007,6,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 6:Step asm */
			1007,9,
			1007,6,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 8:Reset Memory */
			1007,10,
			1007,11,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 9:Reset Registers */
			1007,12,
			1007,13,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 11:Disasm from */
			0,0,
			1007,14,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 12:Dump from */
			0,0,
			1007,15,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 13:Select MPC */
			1007,16,
			1007,17,
			0,0,
			0,0
		}
	}
};

resource 'hmnu' (133, purgeable) {	/* Assembler */
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,

		HMSkipItem { },			/* item -1:Missing menu items */
	{

		HMStringResItem {		/* Menu Title */
			1008,1,
			0,0,
			1004,1,
			1004,2
		},
		HMStringResItem {		/* item 1:Assemble file… */
			1008,2,
			0,0,
			0,0,
			0,0
		},
					HMSkipItem { },
		HMStringResItem {		/* item 3:Preferences… */
			1008,3,
			0,0,
			0,0,
			0,0
		}
	}
};

resource 'hdlg' (263, purgeable) {
	HelpMgrVersion,
	0,
	hmDefaultOptions,
	0,
	0,

		HMSkipItem { },			/* item -1:Missing dialog items */
	{

		HMStringResItem {		/* item 1:OK */
			{0,0},
			{0,0,0,0},
			1009,1,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 2:Cancel */
			{0,0},
			{0,0,0,0},
			1009,2,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 3:DITL 134 */
			{0,0},
			{0,0,0,0},
			1009,3,
			0,0,
			0,0,
			0,0
		},
		HMSkipItem { },			/* item 4:Assembler Prefer */
		HMSkipItem { },			/* item 5:Assemble to memo */
		HMSkipItem { }			/* item 6:DITL… Item 6 */
	}
};

resource 'hdlg' (256, purgeable) {
	HelpMgrVersion,
	0,
	hmDefaultOptions,
	0,
	0,

		HMSkipItem { },			/* item -1:Missing dialog items */
	{

		HMStringResItem {		/* item 1:OK */
			{0,0},
			{0,0,0,0},
			1009,4,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 2:Cancel */
			{0,0},
			{0,0,0,0},
			1009,5,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 3: text insertion */
			{0,0},
			{0,0,0,0},
			1009,6,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 4:Base: */
			{0,0},
			{0,0,0,0},
			0,0,
			0,0,
			1009,7,
			1009,7
		},
		HMSkipItem { }			/* item 5:Value ^0^1: */
	}
};

resource 'hdlg' (262, purgeable) {
	HelpMgrVersion,
	0,
	hmDefaultOptions,
	0,
	0,

		HMSkipItem { },			/* item -1:Missing dialog items */
	{

		HMStringResItem {		/* item 1:OK */
			{0,0},
			{0,0,0,0},
			1009,8,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 2:Cancel */
			{0,0},
			{0,0,0,0},
			1009,9,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 3:Remember Window Positions  */
			{0,0},
			{0,0,0,0},
			1009,10,
			0,0,
			1009,11,
			0,0
		},
		HMStringResItem {		/* item 4:Continuous Dump scrolling */
			{0,0},
			{0,0,0,0},
			1009,12,
			0,0,
			1009,13,
			0,0
		},
		HMStringResItem {		/* item 5:detect ∞ loops */
			{0,0},
			{0,0,0,0},
			1009,14,
			0,0,
			1009,15,
			0,0
		},
		HMStringResItem {		/* item 6: text, PC */
			{0,0},
			{0,0,0,0},
			1009,16,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 7: text, SP */
			{0,0},
			{0,0,0,0},
			1009,17,
			0,0,
			0,0,
			0,0
		},
		HMStringResItem {		/* item 8: text, stack size */
			{0,0},
			{0,0,0,0},
			1009,18,
			0,0,
			0,0,
			0,0
		},
	}
};

