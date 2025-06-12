void Initialize(void);
void DoHelp(void);

main()
{
	Initialize();
	DoHelp();
}




void Initialize(void)
{
	InitGraf(&thePort); 
	InitFonts(); 
	InitWindows(); 
	InitMenus();
	TEInit(); 
	InitDialogs(0L); 
	InitCursor();
}