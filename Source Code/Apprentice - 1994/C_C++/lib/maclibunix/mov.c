/* MPW tool to move a file.
   Public domain by Guido van Rossum, CWI, Amsterdam (July 1987). */

main(argc, argv)
	int argc;
	char **argv;
{
	if (argc != 3) {
		printf("usage: %s oldname newname\n", argv[0]);
		exit(2);
	}
	if (rename(argv[1], argv[2]) != 0) {
		perror(argv[1]);
		exit(1);
	}
	exit(0);
}
