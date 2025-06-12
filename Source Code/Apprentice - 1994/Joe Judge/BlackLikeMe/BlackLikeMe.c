

#include "GraphicsModule_Types.h"
#include "Sounds.h"


// these are the functs that need defined ...
OSErr DoInitialize(Handle *storage, RgnHandle blankRgn, GMParamBlockPtr params);
OSErr DoClose(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params);
OSErr DoBlank(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params);
OSErr DoDrawFrame(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params);
OSErr DoSetUp(RgnHandle blankRgn, short message, GMParamBlockPtr params);

// extra ones
OSErr DoSelected(RgnHandle blankRgn, short message, GMParamBlockPtr params);
OSErr DoAboutBox(RgnHandle blankRgn, short message, GMParamBlockPtr params);



//////////////////////////////////////////////////////////////////////////////////////
// this is the first funct called by AD ... we need to allocate and initialize here
OSErr
DoInitialize(Handle *storage, RgnHandle blankRgn, GMParamBlockPtr params) {

	return noErr;
}

//////////////////////////////////////////////////////////////////////////////////////
// the screen saver has been awakened! time to ditch the storage and wave goodbye
OSErr 
DoClose(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params) {


	return noErr;
}



//////////////////////////////////////////////////////////////////////////////////////
// make the screen go black
OSErr
DoBlank(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params) {

	// darken the screen ...
	FillRgn(blankRgn, params->qdGlobalsCopy->qdBlack);
	return noErr;

}

//////////////////////////////////////////////////////////////////////////////////////
// this is the workhorse routine. It does the continual screen work to make
// this screen saver what it is.
OSErr 
DoDrawFrame(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params) 
{
	// do nothing
	return noErr;
}

//////////////////////////////////////////////////////////////////////////////////////
// this is called when they click on something in the control panel
OSErr 
DoSetUp(RgnHandle blankRgn, short message, GMParamBlockPtr params) {
	// button got pushed?? 
	return noErr;
}



OSErr DoSelected(RgnHandle blankRgn, short message, GMParamBlockPtr params) {
	// this gets called when your module is selected in the AD control panel
	return noErr;
}



OSErr DoAboutBox(RgnHandle blankRgn, short message, GMParamBlockPtr params) {
	// this is called when folks want to see your "about" box
	// but unless you have a "Cals" resource id 1 telling AD that you want this
	// signal, this won't get called.
	return noErr;
}


