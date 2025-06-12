unit HandleTheMenus;

interface

	uses
		AboutIt, Dialogs, Sound, GameUtilities, GliderMain, InitializeAll;

	procedure DoBegintheGame;
	procedure Handle_My_Menu (var doneFlag, Pausing, Playing, gliderCraft, soundOn: boolean; var leftControl, rightControl: char; theMenu, theItem: integer; var theInput: TEHandle);

implementation

{==============================================}

	procedure DoBegintheGame;
	begin
		FlushEvents(EveryEvent, 0);	{Toss out all events								}
		Playing := TRUE;					{Set the bools.									}
		pausing := FALSE;
		switchUntouched := TRUE;
		darkOn := FALSE;
		highScoresOut := FALSE;
		gliderNumber := numberBegin;
		levelOn := levelBegin;
		theScore := 0;
		gameTime := 0;
		totalTime := 0;
		DoFast;
		DisableItem(GetMenu(L_Game), 1);	{Turn off the Begin menu item.			}
		DisableItem(GetMenu(L_Game), 2);	{Turn off Begin in Room... menu item.	}
		EnableItem(GetMenu(L_Game), 4);	{Turn on Pause menu item.				}
		EnableItem(GetMenu(L_Game), 5);	{Turn on End a Game menu item.			}
		DisableItem(GetMenu(L_Options), 0);	{Turn off the entire Apple menu.	}
		ResetGliderPos;							{Get glider in upper left hand corner}
		FoldTheGlider;
		GetSet;									{And get set!!!						}
	end;

{==============================================}

	procedure Handle_My_Menu;
		var
			DNA, index: integer;	{For opening DAs							}
			BoolHolder: boolean;	{For SystemEdit result						}
			DAName: Str255;		{For getting DA name						}
			SavePort: GrafPtr;	{Save current port when opening DAs	}
			tempRect: Rect;		{For warning in Sound_On					}

	begin
		case theMenu of		{Do selected menu list						}
			L_Apple: 
				begin
					case theItem of	{Handle all commands in this menu list	}
						C_About_Glider_2_0: 
							begin
								DisableItem(GetMenu(L_Apple), 0);			{Turn off all the menus				}
								DisableItem(GetMenu(L_Game), 0);
								DisableItem(GetMenu(L_Options), 0);
								DrawMenuBar;
								Open_AboutIt;
							end;
						otherwise										{Handle the DAs						}
							begin
								GetPort(SavePort);							{Save the current port				}
								GetItem(AppleMenu, theItem, DAName);	{Get the name of the DA selected	}
								DNA := OpenDeskAcc(DAName);				{Open the DA selected				}
								SetPort(SavePort);							{Restore to the saved port			}
							end;
					end;
				end;

			L_Game: 
				begin
					case theItem of					{Handle all commands in the Game menu list	}
						C_Start_Game: 
							begin
								DoBegintheGame;
							end;

						C_Start_in_Room: 
							begin
								D_BeginWhere(levelBegin);	{Call the dialog				}
								levelOn := levelBegin;		{When back from dialog...	}
								UpDate_GliderMain;			{update the main window		}
								DoBegintheGame;				{and then begin the game.	}
							end;

						C_Pause: 
							begin
								Pausing := not Pausing;		{Toggle the bool variable.	}
								if Pausing then				{We change the menu item	}
									begin						{to Continue while paused	}
										SetItem(GetMenu(L_Game), theItem, 'Continue');
										SetPort(mainWindow);
										ShowCursor;				{Un-obscure the cursor		}
									end
								else
									begin
										ObscureCursor;			{Rehide the cursor.  And	}
										SetItem(GetMenu(l_Game), theItem, 'Pause');
									end;							{put Pause back into menu.	}
							end;

						C_End_Game: 
							begin
								DoFinalScore;					{Game is aborted, but program not Quit.	}
							end;

						C_Quit: 
							begin
								Playing := FALSE;
								doneFlag := TRUE;
							end;
						otherwise
							begin
							end;
					end;						{End of item case}
				end;						{End for this list}

			L_Options: 
				begin
					case theItem of		{Handle all commands in the Option menu list}
						C_Glider: 
							begin
								gliderCraft := TRUE;
								CheckItem(GetMenu(L_Options), 1, TRUE);		{Put the checkmarks in	 and out	}
								CheckItem(GetMenu(L_Options), 2, FALSE);	{of the proper menu items.			}
								thrust := gliderForwardFast;					{Change glide ratio					}
								stall := gliderBackwardFast;
							end;
						C_Dart: 
							begin
								gliderCraft := FALSE;
								CheckItem(GetMenu(L_Options), 2, TRUE);
								CheckItem(GetMenu(L_Options), 1, FALSE);
								thrust := dartForwardFast;
								stall := dartBackwardFast;
							end;
						C_Sound_On: 
							begin
								soundOn := not soundOn;
								CheckItem(GetMenu(L_Options), C_Sound_On, soundOn);	{If on, put a checkmark		}
							end;
						C_Number_of_Gliders: 
							begin
								D_Number_of_Gliders(numberBegin);			{Call this dialog				}
							end;
						C_Controls: 
							begin
								D_Controls(leftControl, rightControl);			{And call this dialog			}
								scoresChanged := TRUE;
							end;
						C_Fast: 
							begin
								gameSpeed := fastSpeed;
								CheckItem(GetMenu(L_Options), C_Fast, TRUE);	{If on, put a checkmark		}
								CheckItem(GetMenu(L_Options), C_Slow, FALSE);	{If on, put a checkmark		}
							end;
						C_Slow: 
							begin
								gameSpeed := slowSpeed;
								CheckItem(GetMenu(L_Options), C_Slow, TRUE);	{If on, put a checkmark		}
								CheckItem(GetMenu(L_Options), C_Fast, FALSE);	{If on, put a checkmark		}
							end;
						otherwise
							begin
							end;
					end;
				end;														{End for this list				}
			otherwise
				begin
				end;
		end;															{End for lists					}
		HiliteMenu(0);												{Turn menu selection off	}
	end;

{==============================================}

end.