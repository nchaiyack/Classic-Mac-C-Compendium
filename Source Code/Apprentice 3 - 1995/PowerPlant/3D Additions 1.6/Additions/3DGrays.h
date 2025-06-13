// ============================================================================
//	3DGrays.h		   ©1995 J. Rodden, DD/MF & Associates. All rights reserved
// ============================================================================
// Constant definitions of the 16 basic grays.

#pragma once

// 8Bit gray shade constants
const	RGBColor	kGray0	= { 0x0000, 0x0000, 0x0000 };
const	RGBColor	kGray1	= { 0x1111, 0x1111, 0x1111 };
const	RGBColor	kGray2	= { 0x2222, 0x2222, 0x2222 };
const	RGBColor	kGray3	= { 0x3333, 0x3333, 0x3333 };	//
const	RGBColor	kGray4	= { 0x4444, 0x4444, 0x4444 };
const	RGBColor	kGray5	= { 0x5555, 0x5555, 0x5555 };
const	RGBColor	kGray6	= { 0x6666, 0x6666, 0x6666 };	//
const	RGBColor	kGray7	= { 0x7777, 0x7777, 0x7777 };
const	RGBColor	kGray8	= { 0x8888, 0x8888, 0x8888 };	// MediumGray
const	RGBColor	kGray9	= { 0x9999, 0x9999, 0x9999 };	//
const	RGBColor	kGrayA	= { 0xAAAA, 0xAAAA, 0xAAAA };	// MediumLightGray
const	RGBColor	kGrayB	= { 0xBBBB, 0xBBBB, 0xBBBB };	// LightGray4
const	RGBColor	kGrayC	= { 0xCCCC, 0xCCCC, 0xCCCC };
const	RGBColor	kGrayD	= { 0xDDDD, 0xDDDD, 0xDDDD };	// LightGray2
const	RGBColor	kGrayE	= { 0xEEEE, 0xEEEE, 0xEEEE };	// LightGray
const	RGBColor	kGrayF	= { 0xFFFF, 0xFFFF, 0xFFFF };


const	RGBColor	kBlack  = kGray0;			// For convienience...
const	RGBColor	kWhite  = kGrayF;

const	RGBColor	kBackgroundGray	= kGrayE;	// Background color recommended in Develop #15
const	RGBColor	kShadowGray		= kGrayA;	// Shadow color recommened in Develop #15

const	short		k3DMinBitDepth	= 4;
