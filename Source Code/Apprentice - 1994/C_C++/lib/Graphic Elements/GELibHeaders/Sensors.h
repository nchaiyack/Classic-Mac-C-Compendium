/*
	Sensors.h
	
	Basic Graphic Elements which interact with the user. 
	
	For Graphic Elements release version 1.0b1
	
	Copyright 1994 by Al Evans. All rights reserved.
	
	3/7/94
*/

#ifndef GESENSORS
#define GESENSORS

#include "GraphElements.h"

//Sensor types
enum {
	btnSensor,
	switchSensor,
	hSlideSensor,
	vSlideSensor
};

//Sensor states
enum {
	sensorOff = 0,
	sensorOn
};

#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------------------------
//Pushbutton-type sensor
//-------------------------------------------------------------------------------------

GrafElPtr NewButtonSensor(GEWorldPtr world, OSType id, short plane, 
					short resNum, short xPos, short yPos);

//Tracking proc for button-type sensor
pascal Boolean TrackButtonSensor(GEWorldPtr world, GrafElPtr sensor);

//-------------------------------------------------------------------------------------
//Switch-type sensor
//-------------------------------------------------------------------------------------

GrafElPtr NewSwitchSensor(GEWorldPtr world, OSType id, short plane, 
					short resNum, short xPos, short yPos);
					
//Tracking proc for switch-type sensor
pascal Boolean TrackSwitchSensor(GEWorldPtr world, GrafElPtr sensor);

//Set switch to "on" or "off"
pascal void SetSwitchState(GEWorldPtr world, OSType id, short newState);

//-------------------------------------------------------------------------------------
//Slider-type sensor
//-------------------------------------------------------------------------------------

GrafElPtr NewSliderSensor(GEWorldPtr world, OSType id, short plane,
					short resNum, short xPos, short yPos, short sliderType,
					short handleResNum);

//Tracking proc for slider-type sensor
pascal Boolean TrackSliderSensor(GEWorldPtr world, GrafElPtr sensor);

//Set reading of slider-type sensor
pascal void SetSliderPercent(GEWorldPtr world, OSType id, short newSetting);


#ifdef __cplusplus
}
#endif


#endif