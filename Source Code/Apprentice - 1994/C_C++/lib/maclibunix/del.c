/* MPW tool to delete a file.
   Public domain by Guido van Rossum, CWI, Amsterdam (July 1987). */

main(argc, argv)
	int argc;
	char **argv;
{
	if (argc != 2) {
		printf("usage: %s file\n", argv[0]);
		exit(2);
	}
	if (unlink(argv[1]) != 0) {
		perror(argv[1]);
		exit(1);
	}
	exit(0);
}
