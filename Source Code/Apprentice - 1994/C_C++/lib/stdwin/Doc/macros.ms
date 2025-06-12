.\"
.\" Macro definitions and other settings
.\" ------------------------------------
.\"
.\"	The Constant Width font is whatever is mounted at font position 5.
.\"	XXX This is not effective, -ms overrides it (in .TL?),
.\"	but the effect is the same.
.\"
.fp 5 CW
.\"
.\"	Constant width font - use .cW like .I or .R or .B 
.\"   	When using nroff this translates to .I.
.\"
.de cW
.if t \{\
.	ie \\n(.$ \&\f5\\$1\fP\\$2
.	el .ft 5
.\}
.if n .I \\$1 \\$2
..
.\"
.\"	Inline constant width macros -- use like \*<this\*> to get a
.\"	word or two in constant width.  Equivalent to .cW.
.\"
.ds < \&\f5
.ds > \fP
.\"
.\"	Code segment macros -- use .sC to  start a code segment and .eC
.\"	to end it.  Argument to .sC (L, I or B) is passed on to .DS.
.\"	Tabs are set 8 spaces apart.
.\"
.de sC
.DS \\$1 \\$2
.cW
.ta \w'\0\0\0\0\0\0\0\0'u +\w'\0\0\0\0\0\0\0\0'u
..
.de eC
.R
.DE
..
.\"
.\"	Change the point size and spacing to something more radable:
.\"
.nr PS 12
.nr VS 14
.\"
.\"	Some string definitions specific to Python documents:
.\"
.ds Py \f2Python\fP
.ds Ux \s-2UNIX\s0
