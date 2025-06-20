//
// resource definitions of the standard resources for inclusion in a dropbox
//
type 'TMPL' {
	array ItemArray {
		pstring;
		literal longint;
	};
};

type 'flgs' {
	byte  dontEnterFolders,		EnterFolders;
	byte  dontPassFolders,		PassFolders;
	byte  dontEnterInvisibles,	EnterInvisibles;
	byte  dontPassInvisibles,	PassInvisibles;
};

type 'typs' {
	array TypeArray {
		literal longint;	// File Type to pass on to the dropbox
	};
};

type 'tycr' {
	array TypeArray {
		literal longint;	// File Type to accept
		literal longint;	// File Creator to accept
	};
};
