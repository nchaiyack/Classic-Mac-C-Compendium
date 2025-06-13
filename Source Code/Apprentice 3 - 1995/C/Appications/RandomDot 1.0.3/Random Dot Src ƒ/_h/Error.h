/* Error.h
	by David Phillip Oster October 1994 oster@netcom.com
	for:
	Stuart Inglis singlis@waikato.ac.nz
	Department of Computer Science
	University of Waikato, Hamilton, New Zealand
 */
/* ErrorCodes.
 */
enum{
	eUserCancel = -1,
	eClosedPreferences = -10000,
	eBadPict,
	eBadFileType,
	eBadPGMMagicNumber,
	ePictOnly,
	eNeed7AndColor
};

OSErr TellError(OSErr);
