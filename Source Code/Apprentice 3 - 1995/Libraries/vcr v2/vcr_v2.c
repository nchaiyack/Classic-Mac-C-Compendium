// vcr_v2.c
// Darrell Anderson 6/95
// based on Andy Forsberg's "vcr" and AF & DA's "vcr_lib" (5/94)

#include "vcr_v2.h"
#include "ser_v3.h"

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
OSErr VCR_Open( VCRRefPtr *ref, short port, short config, short timeout ) {
	OSErr err;

	// sanity check!
	if( ref == nil ) return(1);

	// allocate a VCRRef
	*ref = (VCRRefPtr) NewPtr( sizeof(VCRRef) );
	err = MemError();
	if(err) return(err);

	// setup the port
	err = Ser_Open( &((*ref)->serialPort), port, config, timeout );
	if(err) return(err);
	
	return( noErr );
}

//----------------------------------------------------------------
// VCR_Close closes and deallocates the VCRRef structure opened/created
// by VCR_Open.
//----------------------------------------------------------------
// -> ref :: the VCRRefPtr for the port.
// returns noErr if all went well
//----------------------------------------------------------------
OSErr VCR_Close( VCRRefPtr ref ) {
	OSErr err;

	// sanity check!
	if( ref == nil ) return(1);

	// close the port
	err = Ser_Close( ref->serialPort );
	if(err) return(err);
	
	// free up the memory
	DisposePtr( (Ptr)ref );
	
	return( noErr );
}

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
Boolean VCR_Command( VCRRefPtr ref, short command, Ptr data ) {
	OSErr err;
	unsigned char byte;
	
	switch( command ) {
	
		// basic commands
		case PLAY:
		case FF:
		case REW:
		case STOP:
			if( !VCR_AckByte(ref,command) ) return(false);
			break;

		case POWER_ON:
		case POWER_OFF:
			if( !VCR_AckByte(ref,EXP_7) ) return(false);
			if( !VCR_AckByte(ref,command) ) return(false);
			break;

		case EJECT:
			if( !VCR_WriteByte(ref,EJECT) ) return(false);
			if( !VCR_ReadByte(ref,&byte) ) return(false);
			if( byte == NAK ) {
				// no tape in VCR!  ignore EJECT command.
				break;
			} else {
				// wait for completion
				while( !VCR_ReadByte(ref,&byte) )
					;
				if( byte != CASETTE_OUT ) return(false);
				break;
			}
			break;
			
		// play speeds
		case REVERSE_SCAN: 	// -x17
		case REVERSE_FAST: 	// -x3
		case REVERSE_PLAY: 	// -x1
		case REVERSE_SLOW: 	// -x(1/5)
		case STILL:        	//  x0
		case FORWARD_SLOW: 	//  x(1/5)
			//FORWARD_PLAY = PLAY, //  x1
		case FORWARD_FAST: 	//  x2
		case FORWARD_SCAN: 	//  x19 
			if( !VCR_AckByte(ref,command) ) return(false);
			break;

		case FORWARD_STEP: 	//  x(1/30)
		case REVERSE_STEP: 	// -x(1/30)
			if( !VCR_AckByte(ref,command) ) return(false);
			if( !VCR_AckByte(ref,'0') ) return(false); // the speed parameter
			if( !VCR_AckByte(ref,'3') ) return(false);
			if( !VCR_AckByte(ref,'0') ) return(false);
			if( !VCR_AckByte(ref,ENTER) ) return(false);
			break;

		// recording control
		case REC:
		case DUB:
			if( !VCR_AckByte(ref,REC_DUB_REQUEST) ) return(false);
			if( !VCR_AckByte(ref,command) ) return(false);
			break;
		case REC_PAUSE:
		case DUB_PAUSE:
			if( !VCR_AckByte(ref,command) ) return(false);
			break;



		default:
			return(false);
	} // end switch(command)
	
	return( true );
}

//----------------------------------------------------------------
// internal commands
//----------------------------------------------------------------

//----------------------------------------------------------------
// VCR_AckBytes sends a byte to the VCR and waits for an 
// acknowledgement before returning.
//----------------------------------------------------------------
// -> ref :: the VCRRefPtr for the port.
// returns noErr if all went well
//----------------------------------------------------------------
Boolean VCR_AckByte( VCRRefPtr ref, unsigned char byte ) {
	OSErr err;
	unsigned char ack_byte;
	
	// send the byte
	if( !VCR_WriteByte(ref, byte) ) return(false);
		
	// expect the ack!
	if( !VCR_ReadByte(ref, &ack_byte) ) return(false);
	
	// was it an ack?
	return( ack_byte == ACK );
}

//----------------------------------------------------------------
// VCR_WriteByte sends a byte to the VCR.
//----------------------------------------------------------------
// -> ref :: the VCRRefPtr for the port.
// returns true if all went well
//----------------------------------------------------------------
Boolean VCR_WriteByte( VCRRefPtr ref, unsigned char byte ) {
	OSErr err;
	
	// send the byte
	err = Ser_Write( ref->serialPort, 1, (Ptr) &byte );
	return( err == noErr );
}

//----------------------------------------------------------------
// VCR_ReadByte reads a byte from the VCR
//----------------------------------------------------------------
// -> ref :: the VCRRefPtr for the port.
// returns true if all went well
//----------------------------------------------------------------
Boolean VCR_ReadByte( VCRRefPtr ref, unsigned char *byte ) {
	OSErr err;
	
	// receive the byte
	err = Ser_Read( ref->serialPort, 1, (Ptr)byte );
	return( err == noErr );
}



