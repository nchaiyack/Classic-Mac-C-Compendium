//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Keys.h
//|
//| This contains header information relating to the use
//| of key controls in HyperCuber.
//|_________________________________________________________

#pragma once

typedef struct
	{
	char dimension;
	char angle;
	long increment;
	short key_code;
	short modifiers;
	} key_control_struct;
	
#define KeyReturn		0x24
#define KeyDelete		0x33
#define KeyPadEquals	0x51
#define KeyEnter		0x4C
#define KeyPadDiv		0x4B
#define KeyPadMult		0x43
#define KeyPadPlus		0x45
#define KeyPadMinus		0x4E
#define KeyPadDot		0x41
#define KeyPad0			0x52
#define KeyPad1			0x53
#define KeyPad2			0x54
#define KeyPad3			0x55
#define KeyPad4			0x56
#define KeyPad5			0x57
#define KeyPad6			0x58
#define KeyPad7			0x59
#define KeyPad8			0x5B
#define KeyPad9			0x5C

