#pragma once
//
// This is a hack. The constructor creates and opens a file,
// which must then be retrieved by calling operator(). There
// is no destructor. The client application is supposed to
// close the file itself.
//
class outfile
{
	public:
	
		outfile( unsigned char *postfix, OSType inWhichFolder = 'desk');

		short operator()() const;
		long  vRefNum() const;

	private:
		FSSpec theFile;
		short fileno;

		static void set2chars( const int n, unsigned char *two_chars);
		
		static void makeprefix( Str31 thename);
};

inline short outfile::operator()() const
{
	return fileno;
}

inline long  outfile::vRefNum() const
{
	return theFile.vRefNum;
}
