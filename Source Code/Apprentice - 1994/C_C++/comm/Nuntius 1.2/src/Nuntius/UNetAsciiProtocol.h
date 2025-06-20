// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UNetAsciiProtocol.h

#define __UNETASCIIPROTOCOL__

class PTcpStream;
class TProgress;

const long kCommandBufferSize = 256;
const long kReturnLineBufferSize = 256;

class PNetAsciiProtocol
{
	public:
	protected:
		Ptr fRespondLineP; // c string
		short fRespondCode;
		
		void SendCommand(char *cmd);
		// cmd should not be CR/LF terminated, only text as C-string

		void GetRespondLine();
		// returns status code
		
		void SendDotTerminatedText(Handle h);

		// The last char in the handle returned by GetDotTerminatedText
		// is null (for optimized parsing)
		Handle GetDotTerminatedText();
		
		void LogCommands(char *p);
		
		PNetAsciiProtocol();
		void INetAsciiProtocol(long newsServerAddr, short port);
		virtual ~PNetAsciiProtocol();

	private:
		PTcpStream *fMacTcp;
		Ptr fReceiveBufferP;
		Ptr fCurrentReceiveP;
		long fCurrentReceiveLeft;
		long fProgressDelta;
		
		Ptr fTransmitBufferP;
		Ptr fCurrentTransmitP;
		long fCurrentTransmitChars;
		
		void FillReceiveBuffer();
		void CopyChars(Handle h, long noChars, long &curSize);
		char GetChar();
		void GetLine();

		void FlushTransmitBuffer();
		void SendChars(const void *p, long noChars);

		void UpdateProgress();

		long DoGetDotTerminatedText(Handle h);
		void UpdateDotBuffer(Handle h, Ptr &destP);
};
