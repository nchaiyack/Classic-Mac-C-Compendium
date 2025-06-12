Boolean IsKeyDown(unsigned short theKey)
// theKey =  any keyboard scan code, 0-127
{
	unsigned char km[16];

	GetKeys((long *) km);
	return ((km[theKey>>3] >> (theKey & 7)) & 1);
}

Boolean CmdKeyDown() {
	return IsKeyDown(0x37);
}

Boolean OptionKeyDown() {
	return IsKeyDown(0x3A);
}

Boolean ShiftKeyDown() {
	return IsKeyDown(0x38);
}

Boolean CapsKeyDown() {
	return IsKeyDown(0x39);
}

Boolean ControlKeyDown() {
	return IsKeyDown(0x3B);
}