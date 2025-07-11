#pragma segment Output
//**********************************************************************
//	The TOutput class methods.	
//		this object writes the information to the disk
//		later it the write method can be overriden to write to DTM port
//**********************************************************************
// -----------------------------------------------------------------------------------------
//	Initialize the output object...
//		the default for this object is to perform the standard save task.
// -----------------------------------------------------------------------------------------
void TOutput::IOutput(TDocument * doc, short grid)
	{
	fDocument	= doc;
	fGrid			= grid;
	return;
	}

// -----------------------------------------------------------------------------------------
//	Process the Menu Commands
// -----------------------------------------------------------------------------------------
void TOutput::Write(short aRefNum, char * string)
	{}
	
// -----------------------------------------------------------------------------------------
//	Process the Menu Commands
// -----------------------------------------------------------------------------------------
void TFOutput::Write(short aRefNum, char * string)
	{
	long len;
	
	len = (long) strlen(string);
	HLock((Handle) this);
	FSWrite(aRefNum,&len,string);
	HUnlock((Handle) this);
	return;
	}

