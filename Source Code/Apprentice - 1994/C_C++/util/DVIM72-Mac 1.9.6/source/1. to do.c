#if justacomment

  To Do:
  -----

update preview when drawing offscreen?

Support required AppleEvents.

-------------------------------

Printing:
---------


Handle_print ->	| Open_printer ->	PrOpenDoc
				| dvifile ->	| dviinit
								| prtpage ->	| PrOpenPage
												| format page offscreen 
												| PrClosePage
								| dviterm
								| Close_printer ->	| PrCloseDoc
													| PrPicFile



Use Mac screen fonts for PostScript fonts.


Font selection not tied to magsteps.

Why doesn�t magnification 5180 work?  What�s this mag 3096 stuff, some
absolute limit on magnification?  3096 = 720 * 1.2^8.

Store all message strings in resources.
#endif