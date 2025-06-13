// test vcr.c
// Darrell Anderson 6/95

#include "vcr_v2.h"

void main(void) {
	OSErr err;
	VCRRefPtr vcr;
	
	// init
	err = VCR_Open( &vcr, DEFAULT_VCR_PORT, DEFAULT_VCR_CONFIG, DEFAULT_VCR_TIMEOUT );
	while(Button()); while(!Button()); while(Button());

	// power on
	err = VCR_Command( vcr, POWER_ON, nil );
	while(Button()); while(!Button()); while(Button());

	// play
	err = VCR_Command( vcr, PLAY, nil );
	while(Button()); while(!Button()); while(Button());

	// stop
	err = VCR_Command( vcr, STOP, nil );
	while(Button()); while(!Button()); while(Button());

	// rewind
	err = VCR_Command( vcr, REW, nil );
	while(Button()); while(!Button()); while(Button());

	// forward scan
	err = VCR_Command( vcr, FORWARD_SCAN, nil );
	while(Button()); while(!Button()); while(Button());

	// stop
	err = VCR_Command( vcr, STOP, nil );
	while(Button()); while(!Button()); while(Button());

	// power off
	err = VCR_Command( vcr, POWER_OFF, nil );
	while(Button()); while(!Button()); while(Button());
	
	// dispose
	err = VCR_Close( vcr );
}