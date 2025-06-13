#if !defined(__DROPUNIX_MAIN__)
#define __DROPUNIX_MAIN__

/*
** Prototypes:
*/
	int Main(int argc, char ** argv);

/*
** #Defines:
*/
	/*
	** Notice that this #define is INTENTIONAL and is used
	** to convert unix sources as cleanly as possible...
	** Just #include this file at the top of the source file
	** containing main(), and add the library and the resource
	** file to the project. Voila! You have a macintosh drag &
	** drop app!
	*/

	#define main Main

#endif /* __DROPUNIX_MAIN__ */
