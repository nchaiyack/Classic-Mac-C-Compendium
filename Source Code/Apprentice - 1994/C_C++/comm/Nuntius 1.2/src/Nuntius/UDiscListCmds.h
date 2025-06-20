// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UDiscListCmds.h

#define __UDISCLISTCMDS__

class TGroupDoc;


class TOpenArticleCommand : public TCommand
{
	public:
		pascal void DoIt();

		TOpenArticleCommand();
		void IOpenArticleCommand(TGroupDoc *doc, 
														TLongintList *discIndexList, 
														Boolean removeSelection);
		pascal void Free();
	private:
		TGroupDoc *fDoc;
		TLongintList *fDiscIndexList;
		Boolean fRemoveSelection;
};
