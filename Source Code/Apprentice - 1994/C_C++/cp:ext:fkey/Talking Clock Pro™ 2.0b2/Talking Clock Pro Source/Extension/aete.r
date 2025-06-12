#include "SysTypes.r"
#include "Types.r"
#include "AERegistry.r"
#include "AppleEvents.r"
#include "AEObjects.r"

#include "TalkConstants.h"

type 'aete' {
	hex byte;											/*major version in binary-coded */
												/* decimal (BCD)*/
	hex byte;											/*minor version in BCD*/
	integer			Language, english = 0, japanese = 11;													/*language code*/
	integer			Script, roman = 0;								/*script code				*/
	integer = $$Countof(Suites);
	array Suites {
		pstring;										/*human-language name of suite	*/
		pstring;										/*suite description*/
		align word;										/*alignment*/
		literal longint;										/*suite ID*/
		integer;										/*suite level				*/
		integer;										/*suite version*/
		integer = $$Countof(Events);
		array Events {
			pstring;									/*human-language name of event*/
			pstring;									/*event description*/
			align word;									/*alignment*/
			literal longint;									/*event class				*/
			literal longint;									/*event ID			*/
			literal longint						noReply = 'null';							/*reply type				*/
			pstring;									/*reply description						*/
			align word;									/*alignment			*/
			boolean			replyRequired,						/*if the reply is */
						replyOptional;						/* required*/
			boolean			singleItem,						/*if the reply must be a list*/
						listOfItems;						
			boolean			notEnumerated,
						isEnumerated;
			boolean;
			boolean;
			boolean;						
			boolean;
			boolean;
			boolean;
			boolean;
			boolean;
			boolean;
			boolean;
			boolean;
			boolean;
			boolean	nonVerbEvent = 1;						/* event; used by Japanese dialect*/
			literal longint						noParams = 'null'; /*direct param type*/
			pstring;									/*direct param description*/
			align word;									/*alignment*/
			boolean			directParamRequired,						 /*if the direct param is required*/
						directParamOptional;
			boolean 			singleItem,						/*if the param must be a list*/
						listOfItems;
			boolean 			notEnumerated,						/*if the type is enumerated*/
						enumerated;
			boolean			doesntChangeState,						 /*if the event changes server's state												*/
						changesState;
			boolean;						/*these 12 bits are reserved;									 */
			boolean;						/* set them to "reserved"		*/
			boolean;						
			boolean;
			boolean;
			boolean;
			boolean;
			boolean;
			boolean;
			boolean;
			boolean;
			boolean;
			integer = $$Countof(OtherParams);
			array OtherParams {
				pstring;								/*human-language name for parameter*/
				align word;								/*alignment*/
				literal longint;								/*parameter keyword*/
				literal longint;								/*parameter type*/
				pstring;								/*parameter description*/
				align word;								/*alignment*/
				boolean			required,					/*if param is required*/
							optional;

				boolean 			singleItem,					/*if the param must be a list*/
							listOfItems;
				boolean 			notEnumerated,					/*if the type is enumerated									*/
							enumerated;
				boolean			isNamed,					/*indicates if this should be the */
							isUnnamed;					/* unnamed parameter; only one										 */
												/* parameter can be so marked; set to */
												/* reserved if not required*/
				boolean;					/*these 9 bits are reserved									; */
				boolean;					/* set them to "reserved"*/
				boolean;
				boolean;
				boolean;
				boolean;
				boolean;
				boolean;
				boolean;
				boolean			notFeminine,					/*feminine; set to reserved if not 												*/
							feminine;					/* required*/
				boolean			notMasculine,					/*masculine			; set to reserved if not								 */
							masculine;					/* required*/
				boolean			singular,
							plural;					/*plural*/
			};
		};
		integer = $$Countof(Classes);
		array Classes {
			pstring;									/*human-language name for class*/
			align word;									/*alignment*/
			literal longint;									/*class ID*/
			pstring;									/*class description						*/
			align word;									/*alignment			*/
			integer = $$Countof(Properties);
			array Properties {
				pstring;								/*human-language name for property*/
				align word;								/*alignment			*/
				literal longint;								/*property ID				*/
				literal longint;								/*property class					*/
				pstring;								/*property description*/
				align word;								/*alignment			*/
				boolean;					/*reserved*/
				boolean			singleItem,					/*if the property must be a list										*/
							listOfItems;					
				boolean			notEnumerated,					/*if the type is					 enumerated*/
							enumerated;				
				boolean			readOnly,					/*can only read it*/
							readWrite;					/*can read or write it*/
				boolean;					/*these 9 bits are reserved; */
				boolean;					/* set them to "reserved"								*/
				boolean;			
				boolean;
				boolean;
				boolean;
				boolean;
				boolean;
				boolean;					
				boolean			notFeminine,					/*feminine; set to reserved if not 												*/
							feminine;					/* required*/
				boolean			notMasculine,					/*masculine			; set to reserved if not								 */
							masculine;					/* required*/
				boolean			singular,
							plural;					/*plural*/
			};
			integer = $$Countof(Elements);
			array Elements {
				literal longint;								/*element class*/
				integer = $$Countof(KeyForms);
				array KeyForms {								/*list of key forms						*/
					literal longint ;
				};
			};
		};
		integer = $$Countof(ComparisonOps);
		array ComparisonOps {
			pstring;									/*human-language name for */
												/* comparison operator			*/
			align word;									/*alignment*/
			literal longint;									/*comparison operator ID								*/
			pstring;									/*comparison operator description*/
			align word;									/*alignment*/
		};
		integer = $$Countof(Enumerations);
		array Enumerations {										/*list of enumerations*/
			literal longint;									/*enumeration ID*/
			integer = $$Countof(Enumerators);
			array Enumerators {									/*list of enumerators*/
				pstring;								/*human-language name for enumerator	*/
				align word;								/*alignment*/
				literal longint;								/*enumerator ID					*/
				pstring;								/*enumerator description								*/
				align word;								/*alignment			*/
			};
		};
	};
}; 


resource 'aete' (0) {
	1 , 0 ,
	english , roman ,
	{	/* Suites */

		"Core suite" ,
		"Basic manipulation events" ,
		kAETalkingClockSuite , 1 , 1 ,
		{	/* Events */

			"get",
			"Get the data for an object",
			kAECoreSuite,
			kAEGetData,
			typeWildCard,
			"the data from the object",
			replyRequired,
			singleItem,
			notEnumerated,
			reserved,
			reserved, reserved, reserved, reserved,
			reserved, reserved, reserved, reserved,
			reserved, reserved, reserved, reserved,
			typeObjectSpecifier,
			"the object whose data is to be returned",
			directParamRequired,
			singleItem,
			notEnumerated,
			doesntChangeState,
			reserved, reserved, reserved, reserved,
			reserved, reserved, reserved, reserved,
			reserved, reserved, reserved, reserved,
			{	/* array OtherParams: 1 elements */
				"as",
				keyAERequestedType,
				typeType,
				"the desired type for the data",
				optional,
				listOfItems,
				notEnumerated,
				reserved,
				reserved, reserved, reserved, reserved,
				reserved, reserved, reserved, reserved,
				reserved, reserved, reserved, reserved,
			} ,

			"set",
			"Set the data of an object",
			kAECoreSuite,
			kAESetData,
			typeWildCard,
			"the data to the object",
			replyOptional,
			singleItem,
			notEnumerated,
			reserved,
			reserved, reserved, reserved, reserved,
			reserved, reserved, reserved, reserved,
			reserved, reserved, reserved, reserved,
			typeObjectSpecifier,
			"the object whose data is to be affected",
			directParamRequired,
			singleItem,
			notEnumerated,
			changesState,
			reserved, reserved, reserved, reserved,
			reserved, reserved, reserved, reserved,
			reserved, reserved, reserved, reserved,
			{	/* array OtherParams: 1 elements */
				"to",
				keyAEData,
				typeWildcard,
				"the desired data for the object",
				required,
				singleItem,
				notEnumerated,
				reserved,
				reserved, reserved, reserved, reserved,
				reserved, reserved, reserved, reserved,
				reserved, reserved, reserved, reserved,
			} ,

			"run",
			"Start the application",
			kCoreEventClass,
			kAEOpenApplication,
			typeNull,
			"",
			replyOptional,
			singleItem,
			notEnumerated,
			reserved,
			reserved, reserved, reserved, reserved,
			reserved, reserved, reserved, reserved,
			reserved, reserved, reserved, reserved,
			typeNull,
			"",
			directParamOptional,
			singleItem,
			notEnumerated,
			changesState,
			reserved, reserved, reserved, reserved,
			reserved, reserved, reserved, reserved,
			reserved, reserved, reserved, reserved,
			{	/* array OtherParams */
			} ,

			"quit",
			"Quit the application",
			kCoreEventClass,
			kAEQuitApplication,
			typeNull,
			"",
			replyOptional,
			singleItem,
			notEnumerated,
			reserved,
			reserved, reserved, reserved, reserved,
			reserved, reserved, reserved, reserved,
			reserved, reserved, reserved, reserved,
			typeNull,
			"",
			directParamOptional,
			singleItem,
			notEnumerated,
			changesState,
			reserved, reserved, reserved, reserved,
			reserved, reserved, reserved, reserved,
			reserved, reserved, reserved, reserved,
			{	/* array OtherParams */
			} ,

			"say the time",
			"Speak the current time",
			kAETalkingClockSuite,
			kAESayTime,
			typeNull,
			"",
			replyOptional,
			singleItem,
			notEnumerated,
			reserved,
			reserved, reserved, reserved, reserved,
			reserved, reserved, reserved, reserved,
			reserved, reserved, reserved, reserved,
			typeNull,
			"",
			directParamOptional,
			singleItem,
			notEnumerated,
			doesntChangeState,
			reserved, reserved, reserved, reserved,
			reserved, reserved, reserved, reserved,
			reserved, reserved, reserved, reserved,
			{	/* array OtherParams */
			} ,

			"say the date",
			"Speak the current time",
			kAETalkingClockSuite,
			kAESayDate,
			typeNull,
			"",
			replyOptional,
			singleItem,
			notEnumerated,
			reserved,
			reserved, reserved, reserved, reserved,
			reserved, reserved, reserved, reserved,
			reserved, reserved, reserved, reserved,
			typeNull,
			"",
			directParamOptional,
			singleItem,
			notEnumerated,
			doesntChangeState,
			reserved, reserved, reserved, reserved,
			reserved, reserved, reserved, reserved,
			reserved, reserved, reserved, reserved,
			{	/* array OtherParams */
			} ,
		} ,
		{	/* Classes */
			"Application",
			cApplication,
			"An application program",
			{	/* array Properties: 9 elements */

				/* [7] */
				"turnover",
				pTurnover,
				cLongInteger,
				"the hour turnover 5-minute period",
				reserved,
				singleItem,
				notEnumerated,
				readWrite,
				reserved, reserved, reserved, reserved,
				reserved, reserved, reserved, reserved,
				reserved,
				reserved,
				reserved,
				reserved,

				/* [8] */
				"optionKeyTalk",
				pOptionKeyTalk,
				cBoolean,
				"Does the clock speak when the option key is held down?",
				reserved,
				singleItem,
				notEnumerated,
				readWrite,
				reserved, reserved, reserved, reserved,
				reserved, reserved, reserved, reserved,
				reserved,
				reserved,
				reserved,
				reserved,

				/* [9] */
				"commandKeyTalk",
				pCommandKeyTalk,
				cBoolean,
				"Does the clock speak when the command key is held down?",
				reserved,
				singleItem,
				notEnumerated,
				readWrite,
				reserved, reserved, reserved, reserved,
				reserved, reserved, reserved, reserved,
				reserved,
				reserved,
				reserved,
				reserved,

				/* [10] */
				"controlKeyTalk",
				pControlKeyTalk,
				cBoolean,
				"Does the clock speak when the control key is held down?",
				reserved,
				singleItem,
				notEnumerated,
				readWrite,
				reserved, reserved, reserved, reserved,
				reserved, reserved, reserved, reserved,
				reserved,
				reserved,
				reserved,
				reserved,

				/* [11] */
				"shiftKeyTalk",
				pShiftKeyTalk,
				cBoolean,
				"Does the clock speak when the shift key is held down?",
				reserved,
				singleItem,
				notEnumerated,
				readWrite,
				reserved, reserved, reserved, reserved,
				reserved, reserved, reserved, reserved,
				reserved,
				reserved,
				reserved,
				reserved,

				/* [12] */
				"capsLockKeyTalk",
				pCapsLockKeyTalk,
				cBoolean,
				"Does the clock speak when the caps lock key is held down?",
				reserved,
				singleItem,
				notEnumerated,
				readWrite,
				reserved, reserved, reserved, reserved,
				reserved, reserved, reserved, reserved,
				reserved,
				reserved,
				reserved,
				reserved,

				/* [13] */
				"fiveMinuteTalk",
				pFiveMinuteTalk,
				cBoolean,
				"Does the clock speak every five minutes?",
				reserved,
				singleItem,
				notEnumerated,
				readWrite,
				reserved, reserved, reserved, reserved,
				reserved, reserved, reserved, reserved,
				reserved,
				reserved,
				reserved,
				reserved,

			},
			{	/* array Elements: 0 elements */
			},
		},
		{	/* array ComparisonOps: 0 elements */
		},
		{	/* array Enumerations: 0 elements */
		},
	},
};

