/*
	Terminal 2.0 script

	Script to dial and log into CompuServe (via LUXPAC Luxembourg,
	Bern Switzerland) and to automatically open a new capture file
	to save the session.

	"Log into CIS.s"
*/

int TIMEOUT = 1800;	/* 30 seconds */

/* ----- Send command and wait for reply ------------------------------- */

check(char *command, char *reply, int timeout)
{
	type(command);
	return prompt(reply, timeout);
}

/* ----- Wait for prompt and send command ------------------------------ */

answer(char *hint, char *command)
{
	int result;

	if (result = prompt(hint, TIMEOUT))
		return result;	/* Timeout or cancel */
	pause(60);			/* Don't type immediatly */
	type(command);
	return 0;
}

/* ----- Main program -------------------------------------------------- */

main()
{
	char name[32];
	int year, month, day, hour, minute, second, weekday;
	int go = 1;

	setup(
		2,		/* 1200 baud */
		1,		/* 8 data */
		0,		/* no parity */
		0,		/* 1 stop */
		-1,		/* port: no change */
		-1,		/* DTR: no change */
		0);		/* Handshake: none */
	type("AT\r");
	pause(60);

	while (go) {
		if (check("AT\r", "OK", 60))
			break;
		pause(30);
		/* Modem setup */
/*		if (check("ATB0S7=60S10=14S9=12\r", "OK", 60))
			break;
		pause(60); */
		/* Dial and log into LUXPAC */
		if (check("ATDP0735\r", "CONNECT", TIMEOUT))
			break;
		if (answer("LUXPAC", "N...-0228464510003\r"))
			break;
		/* Now log into CompuServe */
		if (answer("ID", "73720,2200\r"))
			break;
		if (answer("word", "...\r"))
			break;
		go = 0;			/* Ok */
	}
	if (go) {		/* Timeout or cancel */
		setdtr(0);	/* Negate DTR: modem hangs up */
		pause(60);	/* Wait one second */
		setdtr(1);	/* Assert DTR: now back in command mode */
		beep();		/* If error */
		return;
	}

	/* Get Macintosh date and time */

	date(time(), &year, &month, &day, &hour, &minute, &second, &weekday);

	/* Use date and time to build a unique file name */

	format(name, "CIS %04i%02i%02i%02i%02i%02i",
		year, month, day, hour, minute, second);

	/* Start capturing incoming data into that file */

	if (capture(1, name)) {
		beep();		/* If error */
		return;
	}

	/* The following text is displayed and also captured into the file */

	display("*** CIS session %02i-%02i-%04i %02i:%02i:%02i ***\r\r",
		day, month, year, hour, minute, second);
}
