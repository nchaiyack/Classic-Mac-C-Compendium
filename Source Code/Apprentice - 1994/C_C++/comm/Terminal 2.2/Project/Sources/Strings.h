/*
	Terminal 2.2
	"Strings.h"
*/

#define STR_G	128

enum {
	G_CTFILE = 1,		/* "Capture text in file:" */
	G_CAPTURE,			/* "Capture" */
	G_SCBUFFER,			/* "Save capture buffer as:" */
	G_BUFFER,			/* "Buffer" */
	G_SEND,				/* "Send" */
	G_RECEIVE,			/* "Receive" */
	G_SFRECEIVE,		/* "Save file to receive as:" */
	G_DOWNLOAD,			/* "Download" */
	G_TRANSMIT,			/* "Transmit" */
	G_BIN,				/* ".BIN" */
	G_MAKE,				/* "Make" */
	G_MAKENAME,			/* "Output file name" */
	G_EXTRACT,			/* "Extract" */
	G_SFEXTRACT,		/* "Save file extracted as:" */
	G_SETTINGS,			/* "Terminal Settings" */
	G_TERMINAL,			/* "Terminal" */
	G_SELECT,			/* "Select" */
	G_SCRIPT,			/* "Script" */
	G_SUFFIX,			/* ".s" */
	G_SCRIPTFOLDER,		/* "Terminal Scripts" */
	G_MACROS,			/* "Macros.m" */
	G_MSUFFIX,			/* ".m" */
	G_MACRO,			/* "Macros" */
	G_OPEN,				/* "Open" */
	G_SAVE,				/* "Save" */
	G_APPEND,			/* "Apend" */
	G_KISS				/* "Kiss" */
};

#define STR_P 129

enum {
	P_VERIFY = 1,		/* "Verified" */
	P_TIMEOUT,			/* "Timeout" */
	P_INVALID,			/* "Invalid character" */
	P_REPEAT,			/* "Repeated" */
	P_CRCERR,			/* "Wrong checksum/CRC" */
	P_BLOCKERR1,		/* "Block number error" */
	P_BLOCKERR2,		/* "Wrong block number" */
	P_XMODEM,			/* "X-Modem " */
	P_CRC,				/* "CRC " */
	P_CHECKSUM,			/* "checksum " */
	P_TEXT,				/* "(TEXT)" */
	P_BINARY1,			/* "(MacBinary I)" */
	P_BINARY2,			/* "(MacBinary II)" */
	P_STEXT,			/* "Sending text file:" */
	P_RFILE,			/* "Receiving file:" */
	P_TFILE,			/* "Transmitting file:" */
	P_1K,				/* "1K " */
	P_CISB,				/* "CompuServe B file: " */
	P_UPLOAD,			/* "Upload" */
	P_DOWNLOAD,			/* "Download" */
	P_END,				/* "End of transmission" */
	P_REVERIFY,			/* "Re-verified" */
	P_YMODEM,			/* "Y-Modem " */
	P_NEGOCIATE,		/* "Negociating�" */
	P_QUICKB,			/* "QuickB file: */
	P_SENDAHEAD,		/* "Send Ahead " */
	P_SSCRAP			/* "Sending clipboard" */
};

#define STR_M 130

enum {
	M_ERROR = 1,		/* "Error #" */
	M_TRANSFER,			/* "File transfer completed: ..." */
	M_PROGRESS,			/* "File transfer in progress" */
	M_OLD				/* "Unsupported configuration" */
};

#define STR_S 131		/* Error strings from script interpreter */
