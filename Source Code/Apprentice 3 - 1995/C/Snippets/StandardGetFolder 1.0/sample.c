/*******************************************************************************
* StandardGetFolder Sample Program                             by Ken Kirksey  *
*                                                                              *
*    This little program gives an example of how to use the standard get       *
*    folder function.                                                          *
*                                                                              *
*     Requires:  System 7.0 or later.  ANSI & MacTraps libraries.              *
*******************************************************************************/


void main()
{
    StandardFileReply       mySFReply;
    Point                   where = {60,60};
    OSErr                   iErr;
    FSSpec                  myFSSpec;
    char                    buffer[80];
    long                    inOutCount;
    int                     testFile;

    /*-------------------------------------------------------------------------+
    | Initialize all that toolbox stuff.                                       |
    +-------------------------------------------------------------------------*/
    InitGraf( &thePort );
    InitFonts();
    InitWindows();
    FlushEvents( everyEvent, 0 );
    InitCursor();
    InitMenus();
    TEInit();
    InitDialogs( 0L );

    /*-------------------------------------------------------------------------+
    | Call Standard get folder, passing the point you want it to be drawn at,  |
    | the message you want to be displayed above the file list, and a pointer  |
    | to a StandardFileReply record.                                           |
    +-------------------------------------------------------------------------*/
    StandardGetFolder( where, "\pHome Free:", &mySFReply ); 
    
    /*-------------------------------------------------------------------------+
    | Ok, the volume reference number and directory ID of the folder the user  |
    | chose are returned in the sfFile field of the Standard File Reply.  Use  |  
    | use this information to build an FSSpec record that references the file  |
    | you wish to create.                                                      |
    +-------------------------------------------------------------------------*/
    FSMakeFSSpec ( mySFReply.sfFile.vRefNum, 
                   mySFReply.sfFile.parID,
                   "\pMyFile.test",
                   &myFSSpec);
                   
    /*-------------------------------------------------------------------------+
    | Now using the FSSpec record you made, create the file. I've got it set   |
    | to create an Alpha (my favorite text editor) file. Change the creator    |
    | code to suit your taste.                                                 |
    +-------------------------------------------------------------------------*/
    FSpCreate (&myFSSpec, 'ALFA', 'TEXT', mySFReply.sfScript);


    /*-------------------------------------------------------------------------+
    | Open the file and write a short message to it.                           |
    +-------------------------------------------------------------------------*/
    iErr = FSpOpenDF (&myFSSpec, fsRdWrPerm, &testFile);
    if (iErr != noErr);
    SetEOF (testFile, 0);
        
    inOutCount = sprintf (buffer, "Holey Smokes, It Worked!!!\r");
    iErr = FSWrite (testFile, &inOutCount, buffer); 
    if (iErr != noErr);
    
    /*-------------------------------------------------------------------------+
    | Close the file.                                                          |
    +-------------------------------------------------------------------------*/
    FSClose (testFile);
            
}


