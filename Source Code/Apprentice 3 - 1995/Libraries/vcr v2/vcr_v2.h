// vcr_v2.h
// Darrell Anderson 6/95
// based on Andy Forsberg's "vcr" and AF & DA's "vcr_lib" (5/94)

#ifndef __VCR_V2__
#define __VCR_V2__

#include "ser_v3.h"

#define DEFAULT_VCR_PORT quadraLink3
#define DEFAULT_VCR_CONFIG standard_9600
#define DEFAULT_VCR_TIMEOUT DEFAULT_TIMEOUT

enum {
	// basic functions
	PLAY = 0x3A,
	FF = 0xAB,
	REW = 0xAC,
	STOP = 0x3F,
	EJECT = 0x2A,
	POWER_ON = 0xCE,
	POWER_OFF = 0xCF,
	
	// play speeds
	REVERSE_SCAN = 0x4E, 	// -x17
	REVERSE_FAST = 0x4B, 	// -x3
	REVERSE_PLAY = 0x4A, 	// -x1
	REVERSE_SLOW = 0x4C, 	// -x(1/5)
	REVERSE_STEP = 0x4D, 	// -x(1/30) // needs option parameter!
	STILL = 0x4F,        	//  x0
	FORWARD_STEP = 0x3D, 	//  x(1/30) // needs option parameter!
	FORWARD_SLOW = 0x3C, 	//  x(1/5)
	//FORWARD_PLAY = PLAY, //  x1
	FORWARD_FAST = 0x3B, 	//  x2
	FORWARD_SCAN = 0x3E, 	//  x19 
	
	// recording control
	REC = 0xCA, // needs REC_DUB_REQUEST first!
	REC_PAUSE = 0xCB,
	DUB = 0xCC, // needs REC_DUB_REQUEST first!
	DUB_PAUSE = 0xCD,
	REC_DUB_REQUEST = 0xFA,
	
	// detection commands
	TIME_CODE_SENSE,
	STATUS_SENSE_1,
	STATUS_SENSE_2,
	STATUS_SENSE_3,
	ADDR_INQ,
	STATUS_INQ,
	VCR_DATA_SENSE,
	ERROR_SENSE,
	IN_DATA_SENSE,
	OUT_DATA_SENSE,
	DIGITAL_MODE_SENSE,
	U_BIT_SENSE,
	U_BIT_PRESET_SENSE,
	
	// search commands
	TIME_CODE_MARK_SET,
	SEARCH_WITH_DATA,
	TIME_CODE_REPEAT,
	SEARCH,
	REPEAT,
	MEMORY,
	MEMORY_SEARCH,
	
	// audio/video i/o
	AUDIO_MUTE_ON,
	AUDIO_MUTE_OFF,
	VIDEO_MUTE_ON,
	VIDEO_MUTE_OFF,
	AV_MUTE_ON,
	AV_MUTE_OFF,
	
	// digital frame memory control commands
	DIGITAL_MODE,
	DIGITAL_MODE_OFF,
	FRAME_MEMORY_SELECT,
	FRAME_STILL,
	FREEZE_FRAME_OUT_MODE,
	FREEZE_FIELD_OUT_MODE,
	
	// others
	ACK = 0x0A,
	NAK = 0x0B,
	COMPLETION = 0x01,
	ERROR = 0x02,
	NOT_TARGET = 0x05,
	CASETTE_OUT = 0x03,
	MARK_RETURN = 0x07,
	ENTER = 0x40,
	C_E = 0x41,
	C_L = 0x56,
	C_L_AND_INITIALIZE = 0xCF, // requires exp-8
	EXP_7 = 0xDE,
	EXP_8 = 0xDF,
	POWER_EJECT_ENABLE,
	POWER_EJECT_DISABLE,
	VCR_DATA_PRESET,
	CONTROLLER_ENABLE,
	CONTROLLER_DISABLE,
	INDEX_ON,
	INDEX_OFF
};

typedef struct VCRRef {
	// serial port reference for the VCR's connection
	SerRefPtr serialPort;
	
	// status
//	Boolean 
//		power,
//		controllerEnabled,
//		indexEnabled,
//		audioEnabled,
//		videoEnabled,
//		tapePresent;
		
} VCRRef, *VCRRefPtr;

//----------------------------------------------------------------
// VCR_Open allocates and fills in a VCRRef structure, opens and configures
// the appropriate serial port.
//----------------------------------------------------------------
// <- ref :: a pointer to a VCRRefPtr (NOT a VCRRef, b/c it allocates it's own)
//           use this VCRRefPtr for later calls to the VCR.
// -> whichPort :: which serial port we're dealing with (ie modemPort, etc, from above)
//                 DEFAULT_VCR_PORT is defined in the header file.
// -> config :: the configuration parameter (ie standard_9600, etc, from above)
//              DEFAULT_VCR_CONFIG is defined in the header file.
// -> timeout :: how many 1/60ths of a second before a timeout occurs, only
//               applies when reading.  (DEFAULT_VCR_TIMEOUT is defined in the header)
// returns noErr if all went well
//----------------------------------------------------------------
OSErr VCR_Open( VCRRefPtr *ref, short port, short config, short timeout );

//----------------------------------------------------------------
// VCR_Close closes and deallocates the VCRRef structure opened/created
// by VCR_Open.
//----------------------------------------------------------------
// -> ref :: the VCRRefPtr for the port.
// returns noErr if all went well
//----------------------------------------------------------------
OSErr VCR_Close( VCRRefPtr ref );

//----------------------------------------------------------------
// VCR_Command issues a command to the vcr, awaiting completion before
// returning.  Some commands require additional info, passed in via the
// 'data' parameter.  If the command does not need it, pass 'nil'
//----------------------------------------------------------------
// -> ref :: the VCRRefPtr for the port.
// -> command :: the command we wish to execute (see header file for list)
// -> data :: pass a pointer to further data if appropriate, nil otherwise.
// returns true if all went well
//----------------------------------------------------------------
Boolean VCR_Command( VCRRefPtr ref, short command, Ptr data );

//----------------------------------------------------------------
// internal commands
//----------------------------------------------------------------

//----------------------------------------------------------------
// VCR_AckByte sends a byte to the VCR and waits for an 
// acknowledgement before returning.
//----------------------------------------------------------------
// -> ref :: the VCRRefPtr for the port.
// returns true if all went well
//----------------------------------------------------------------
Boolean VCR_AckByte( VCRRefPtr ref, unsigned char byte );

//----------------------------------------------------------------
// VCR_WriteByte sends a byte to the VCR.
//----------------------------------------------------------------
// -> ref :: the VCRRefPtr for the port.
// returns true if all went well
//----------------------------------------------------------------
Boolean VCR_WriteByte( VCRRefPtr ref, unsigned char byte );

//----------------------------------------------------------------
// VCR_ReadByte reads a byte from the VCR
//----------------------------------------------------------------
// -> ref :: the VCRRefPtr for the port.
// returns true if all went well
//----------------------------------------------------------------
Boolean VCR_ReadByte( VCRRefPtr ref, unsigned char *byte ); 

#endif
