#include <AppleTalk.h>
#include <stream.h>

int main()
{
	MPPParamBlock pb;
	pb.SETSELF.csCode = setSelfSend;
	pb.SETSELF.newSelfFlag = 1;
	OSErr err = PSetSelfSend(&pb, false); 
	if (err)
	{
		cerr << "Got error = " << err << "\n";
		return 1;
	}
	return 0;
}