/* Progress.h
	by David Phillip Oster October 1994 oster@netcom.com
	for:
	Stuart Inglis singlis@waikato.ac.nz
	Department of Computer Science
	University of Waikato, Hamilton, New Zealand
 */
void	SetProgressLabelText(StringPtr s);
OSErr	ShowProgress(LongInt n, LongInt max);
void 	RecordProgress(void);
