/* CSack.c */

#include "CSack.h"
#include "Memory.h"


void				CSack::PushElement(void* Element)
	{
		SackBlock**			Temp;
		register short	Scan;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CSack::PushElement called on uninitialized object."));
		EXECUTE(Scannable = False;)
		CheckHandleExistence((Handle)FirstBlock);
		if (((**FirstBlock).NumBytes) < MaxBytesPerBlock)
			{
				Temp = (SackBlock**)AllocHandle(((**FirstBlock).NumBytes
					+ BytesPerElement) + sizeof(SackBlock));
				SetTag(Temp,"CSack Block");
				(**Temp).Next = (**FirstBlock).Next;
				(**Temp).NumElements = (**FirstBlock).NumElements + 1;
				(**Temp).NumBytes = (**FirstBlock).NumBytes + BytesPerElement;
				ERROR((**Temp).NumBytes % BytesPerElement != 0,PRERR(ForceAbort,
					"CSack::PushElement element array size inconsistency"));
				HRNGCHK(Temp,&((**Temp).Data[BytesPerElement]),(**FirstBlock).NumBytes);
				HRNGCHK(FirstBlock,&((**FirstBlock).Data[0]),(**FirstBlock).NumBytes);
				MemCpy(&((**Temp).Data[BytesPerElement]),&((**FirstBlock).Data[0]),
					(**FirstBlock).NumBytes);
				HRNGCHK(Temp,&((**Temp).Data[0]),BytesPerElement);
				MemCpy(&((**Temp).Data[0]),Element,BytesPerElement);
				ReleaseHandle((Handle)FirstBlock);
				FirstBlock = Temp;
			}
		 else
			{
				Temp = (SackBlock**)AllocHandle(sizeof(SackBlock) + BytesPerElement);
				SetTag(Temp,"CSack Block");
				(**Temp).Next = FirstBlock;
				(**Temp).NumElements = 1;
				(**Temp).NumBytes = BytesPerElement;
				HRNGCHK(Temp,&((**Temp).Data[0]),BytesPerElement);
				MemCpy(&((**Temp).Data[0]),Element,BytesPerElement);
				FirstBlock = Temp;
			}
	}


MyBoolean		CSack::KillElement(void* Element)
	{
		SackBlock**			Scan;
		SackBlock**			Lag;
		register short	Index;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CSack::KillElement called on uninitialized object."));
		EXECUTE(Scannable = False;)
		Scan = FirstBlock;
		Lag = NIL;
		while (Scan != NIL)
			{
				CheckHandleExistence((Handle)Scan);
				for (Index = 0; Index < (**Scan).NumBytes; Index += BytesPerElement)
					{
						HRNGCHK(Scan,&((**Scan).Data[Index]),BytesPerElement);
						if (MemEqu(Element,&((**Scan).Data[Index]),BytesPerElement))
							{
								SackBlock**			Temp;
								register short	Count;

								Temp = (SackBlock**)AllocHandle((**Scan).NumBytes
									- BytesPerElement + sizeof(SackBlock));
								SetTag(Temp,"CSack Block");
								(**Temp).Next = (**Scan).Next;
								(**Temp).NumElements = (**Scan).NumElements - 1;
								(**Temp).NumBytes = (**Scan).NumBytes - BytesPerElement;
								HRNGCHK(Temp,&((**Temp).Data[0]),Index);
								HRNGCHK(Scan,&((**Scan).Data[0]),Index);
								MemCpy(&((**Temp).Data[0]),&((**Scan).Data[0]),Index);
								HRNGCHK(Temp,&((**Temp).Data[Index]),(**Scan).NumBytes
									- BytesPerElement - Index);
								HRNGCHK(Scan,&((**Scan).Data[Index + BytesPerElement]),
									(**Scan).NumBytes - BytesPerElement - Index);
								MemCpy(&((**Temp).Data[Index]),&((**Scan).Data[Index + BytesPerElement]),
									(**Scan).NumBytes - BytesPerElement - Index);
								if (Lag == NIL)
									{
										FirstBlock = Temp;
									}
								 else
									{
										CheckHandleExistence((Handle)Lag);
										(**Lag).Next = Temp;
									}
								ERROR((**Temp).NumBytes % BytesPerElement != 0,PRERR(ForceAbort,
									"CSack::KillElement element array size inconsistency"));
								ReleaseHandle((Handle)Scan);
								if (((**Temp).NumElements == 0) && (Temp != FirstBlock))
									{
										(**Lag).Next = (**Temp).Next;
										ReleaseHandle((Handle)Temp);
									}
								return True; /* return after deletion of one */
							}
					}
				Lag = Scan;
				Scan = (**Scan).Next;
			}
		return False;
	}


void				CSack::ResetScan(void)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CSack::ResetScan called on uninitialized object."));
		NextHandleToAccess = FirstBlock;
		NextIndexToAccess = -BytesPerElement; /* start out one before */
		EXECUTE(Scannable = True;)
	}


MyBoolean		CSack::GetNext(void* PlaceToPut)
	{
		register short		Scan;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CSack::GetNext called on uninitialized object."));
		ERROR(!Scannable,PRERR(ForceAbort,
			"CSack::GetNext called on unscannable object."));
	 RestartPoint:
		if (NextHandleToAccess == NIL)
			{
				return False;
			}
		CheckHandleExistence((Handle)NextHandleToAccess);
		ERROR(NextIndexToAccess % BytesPerElement != 0,PRERR(ForceAbort,
			"CSack::GetNext NextIndexToAccess inconsistency"));
		ERROR((**NextHandleToAccess).NumBytes % BytesPerElement != 0,PRERR(ForceAbort,
			"CSack::GetNext element array size inconsistency"));
		NextIndexToAccess += BytesPerElement;
		if (NextIndexToAccess >= (**NextHandleToAccess).NumBytes)
			{
				NextHandleToAccess = (**NextHandleToAccess).Next;
				NextIndexToAccess = -BytesPerElement;
				goto RestartPoint;
			}
		HRNGCHK(NextHandleToAccess,&((**NextHandleToAccess).Data[NextIndexToAccess]),
			BytesPerElement);
		MemCpy(PlaceToPut,&((**NextHandleToAccess).Data[NextIndexToAccess]),BytesPerElement);
		return True;
	}


ulong				CSack::NumElements(void)
	{
		register ulong				Accr;
		register SackBlock**	Scan;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CSack::NumElements called on uninitialized object."));
		Accr = 0;
		Scan = FirstBlock;
		while (Scan != NIL)
			{
				CheckHandleExistence((Handle)Scan);
				ERROR((**Scan).NumBytes % BytesPerElement != 0,PRERR(ForceAbort,
					"CSack::GetNext element array size inconsistency"));
				Accr += (**Scan).NumElements;
				Scan = (**Scan).Next;
			}
		return Accr;
	}


/* */				CSack::CSack()
	{
		SackBlock**			Shadow;

		SetTag(this,"CSack");
		Shadow = (SackBlock**)AllocHandle(sizeof(SackBlock));
		SetTag(Shadow,"CSack Block");
		(**Shadow).Next = NIL;
		(**Shadow).NumElements = 0;
		(**Shadow).NumBytes = 0;
		FirstBlock = Shadow;
		EXECUTE(Scannable = False;)
	}


/* */				CSack::~CSack()
	{
		register SackBlock**	FirstImage;
		register SackBlock**	Temp;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CSack::~CSack called on uninitialized object."));
		FirstImage = FirstBlock;
		while (FirstImage != NIL)
			{
				Temp = FirstImage;
				FirstImage = (**FirstImage).Next;
				ReleaseHandle((Handle)Temp);
			}
	}


void				CSack::ISack(long TheSizeOfElement, long TheMaxElementsPerBlock)
	{
		ERROR(Initialized == True,PRERR(ForceAbort,
			"CSack::ISack called on already initialized object."));
		EXECUTE(Initialized = True);
		BytesPerElement = TheSizeOfElement;
		MaxBytesPerBlock = TheMaxElementsPerBlock * TheSizeOfElement;
		ERROR((BytesPerElement<0)||(BytesPerElement>32767),PRERR(ForceAbort,
			"CSack::ISack BytesPerElement initializer is out of range."));
		ERROR((MaxBytesPerBlock<BytesPerElement)||(MaxBytesPerBlock>32767),PRERR(ForceAbort,
			"CSack::ISack MaxBytesPerBlock initializer is out of range."));
		EXECUTE(MaxBytesPerBlock = TheSizeOfElement * 4;) /* for error testing */
	}


MyBoolean		CSack::GetCurrent(void* PlaceToPut)
	{
		register short		Scan;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CSack::GetCurrent called on uninitialized object."));
		ERROR(!Scannable,PRERR(ForceAbort,
			"CSack::GetCurrent called on unscannable object."));
	 RestartPoint:
		if ((NextHandleToAccess == NIL) || (NextIndexToAccess < 0))
			{
				return False;
			}
		HRNGCHK(NextHandleToAccess,&((**NextHandleToAccess).Data[NextIndexToAccess]),
			BytesPerElement);
		MemCpy(PlaceToPut,&((**NextHandleToAccess).Data[NextIndexToAccess]),BytesPerElement);
		return True;
	}


MyBoolean		CSack::AdvanceToNext(void)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CSack::AdvanceToNext called on uninitialized object."));
		ERROR(!Scannable,PRERR(ForceAbort,
			"CSack::AdvanceToNext called on unscannable object."));
	 RestartPoint:
		if (NextHandleToAccess == NIL)
			{
				return False;
			}
		NextIndexToAccess += BytesPerElement;
		if (NextIndexToAccess >= (**NextHandleToAccess).NumBytes)
			{
				NextHandleToAccess = (**NextHandleToAccess).Next;
				NextIndexToAccess = 0;
				goto RestartPoint;
			}
		return True;
	}


/* insert after any that return negative or zero number, but before one that */
/* returns a positive number (post-insertion sort) which allows us to use this */
/* to implement a stable sort. */
void				CSack::InsertSorted(void* Element, short (*Comparator)(void*,void*))
	{
		SackBlock**		SackScan;
		SackBlock**		Lag;
		SackBlock**		LookAhead;
		long					Index;
		SackBlock**		Temp;
		long					Count;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CSack::InsertSorted called on uninitialized object."));
		EXECUTE(Scannable = False;)
		/* First, find which block it should be inserted into. */
	 RetryPoint:
		SackScan = FirstBlock;
		if (SackScan == NIL)
			{
				/* use the normal routine to add an element to an empty list */
				PushElement(Element);
				return;
			}
		Lag = NIL;
		LookAhead = (**SackScan).Next;
		while ((LookAhead != NIL) && (HLock((Handle)LookAhead),
			(*Comparator)(Element,&((**LookAhead).Data[0])) >= 0))
			{
				HUnlock((Handle)LookAhead);
				Lag = SackScan;
				SackScan = LookAhead;
				LookAhead = (**SackScan).Next;
			}
		if (LookAhead != NIL) {HUnlock((Handle)LookAhead);}
		/* if the block is too big, we have to break it into two. */
		if ((**SackScan).NumBytes + BytesPerElement >= MaxBytesPerBlock)
			{
				SackBlock**		FirstHalf;
				SackBlock**		SecondHalf;
				long					ElementsInFirstHalf;
				long					ElementsInSecondHalf;
				long					Scan;

				ElementsInFirstHalf = (**SackScan).NumElements / 2;
				ElementsInSecondHalf = (**SackScan).NumElements - ElementsInFirstHalf;
				FirstHalf = (SackBlock**)AllocHandle(sizeof(SackBlock)
					+ (ElementsInFirstHalf * BytesPerElement));
				SetTag(FirstHalf,"CSack Block");
				SecondHalf = (SackBlock**)AllocHandle(sizeof(SackBlock)
					+ (ElementsInSecondHalf * BytesPerElement));
				SetTag(SecondHalf,"CSack Block");
				if (Lag == NIL)
					{
						FirstBlock = FirstHalf;
					}
				 else
					{
						(**Lag).Next = FirstHalf;
					}
				(**FirstHalf).Next = SecondHalf;
				(**FirstHalf).NumElements = ElementsInFirstHalf;
				(**FirstHalf).NumBytes = ElementsInFirstHalf * BytesPerElement;
				(**SecondHalf).Next = (**SackScan).Next;
				(**SecondHalf).NumElements = ElementsInSecondHalf;
				(**SecondHalf).NumBytes = ElementsInSecondHalf * BytesPerElement;
				HRNGCHK(FirstHalf,&((**FirstHalf).Data[0]),(**FirstHalf).NumBytes);
				HRNGCHK(SackScan,&((**SackScan).Data[0]),(**FirstHalf).NumBytes);
				MemCpy(&((**FirstHalf).Data[0]),&((**SackScan).Data[0]),(**FirstHalf).NumBytes);
				HRNGCHK(SecondHalf,&((**SecondHalf).Data[0]),(**SecondHalf).NumBytes);
				HRNGCHK(SackScan,&((**SackScan).Data[(**FirstHalf).NumBytes]),
					(**SecondHalf).NumBytes);
				MemCpy(&((**SecondHalf).Data[0]),&((**SackScan).Data[(**FirstHalf).NumBytes]),
					(**SecondHalf).NumBytes);
				ReleaseHandle((Handle)SackScan);
				/* go back and make sure we are still inserting into proper block */
				goto RetryPoint;
			}
		/* Now we have the block (SackScan) so where to insert into it? */
		HLock((Handle)SackScan);
		Index = 0;
		/* We can only insert in the <Index> position if the element before */
		/* us is less than or equal to us and the element after us is greater */
		/* than us.  If Index == 0, then there is no element before us. */
		/* The first scan works so that we won't ever insert before the first */
		/* element (it stops before the first element is equal to or greater than */
		/* us).  Thus, we check the one that will be after us and advance until it is */
		/* greater.  If we hit the end, we append. */
		while ((Index < (**SackScan).NumBytes) && ((*Comparator)(Element,
			&((**SackScan).Data[Index])) >= 0))
			{
				/* Scanning... */
				Index += BytesPerElement;
			}
		HUnlock((Handle)SackScan);
		/* if (Index == NumBytes), then we append, else we insert */
		/* the algorithm is the same for either one... */
		Temp = (SackBlock**)AllocHandle(HandleSize((Handle)SackScan) + BytesPerElement);
		SetTag(Temp,"CSack Block");
		(**Temp).Next = (**SackScan).Next;
		(**Temp).NumElements = (**SackScan).NumElements + 1;
		(**Temp).NumBytes = (**SackScan).NumBytes + BytesPerElement;
		if (Lag == NIL)
			{
				FirstBlock = Temp;
			}
		 else
			{
				(**Lag).Next = Temp;
			}
		/* copying over preceding zone */
		HRNGCHK(Temp,&((**Temp).Data[0]),Index);
		HRNGCHK(SackScan,&((**SackScan).Data[0]),Index);
		MemCpy(&((**Temp).Data[0]),&((**SackScan).Data[0]),Index);
		/* copying over element */
		HRNGCHK(Temp,&((**Temp).Data[Index]),BytesPerElement);
		MemCpy(&((**Temp).Data[Index]),Element,BytesPerElement);
		/* copying over succeeding range (does nothing if Index == NumLongs) */
		HRNGCHK(Temp,&((**Temp).Data[Index + BytesPerElement]),(**SackScan).NumBytes - Index);
		HRNGCHK(SackScan,&((**SackScan).Data[Index]),(**SackScan).NumBytes - Index);
		MemCpy(&((**Temp).Data[Index + BytesPerElement]),&((**SackScan).Data[Index]),
			(**SackScan).NumBytes - Index);
		ReleaseHandle((Handle)SackScan);
	}


void				CSack::KillCurrent(void)
	{
		SackBlock**			Scan;
		SackBlock**			Lag;
		SackBlock**			Temp;
		register short	Count;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CSack::KillElement called on uninitialized object."));
		ERROR(!Scannable,PRERR(ForceAbort,
			"CSack::KillCurrent called on unscannable object."));
		/* this function preserves scannability */
		if ((NextIndexToAccess < 0) || (NextHandleToAccess == NIL))
			{
				return; /* couldn't do it */
			}
		Scan = FirstBlock;
		Lag = NIL;
		while (Scan != NextHandleToAccess)
			{
				Lag = Scan;
				Scan = (**Scan).Next;
			}
		ERROR(Scan == NIL,PRERR(ForceAbort,"CSack internal list inconsistency"));
		Temp = (SackBlock**)AllocHandle((**Scan).NumBytes - BytesPerElement + sizeof(SackBlock));
		SetTag(Temp,"CSack Block");
		(**Temp).Next = (**Scan).Next;
		(**Temp).NumElements = (**Scan).NumElements - 1;
		(**Temp).NumBytes = (**Scan).NumBytes - BytesPerElement;
		HRNGCHK(Temp,&((**Temp).Data[0]),NextIndexToAccess);
		HRNGCHK(Scan,&((**Scan).Data[0]),NextIndexToAccess);
		MemCpy(&((**Temp).Data[0]),&((**Scan).Data[0]),NextIndexToAccess);
		HRNGCHK(Temp,&((**Temp).Data[NextIndexToAccess]),
			(**Scan).NumBytes - BytesPerElement - NextIndexToAccess);
		HRNGCHK(Scan,&((**Scan).Data[NextIndexToAccess + BytesPerElement]),
			(**Scan).NumBytes - BytesPerElement - NextIndexToAccess);
		MemCpy(&((**Temp).Data[NextIndexToAccess]),&((**Scan).Data[NextIndexToAccess
			+ BytesPerElement]),(**Scan).NumBytes - BytesPerElement - NextIndexToAccess);
		if (Lag == NIL)
			{
				FirstBlock = Temp;
			}
		 else
			{
				CheckHandleExistence((Handle)Lag);
				(**Lag).Next = Temp;
			}
		ReleaseHandle((Handle)Scan);
		if (((**Temp).NumElements == 0) && (Temp != FirstBlock))
			{
				(**Lag).Next = (**Temp).Next;
				ReleaseHandle((Handle)Temp);
				Temp = (**Lag).Next;
			}
		NextHandleToAccess = Temp;
		/* now check to see if that was the last in handle, and if so, advance to next */
	 RestartPoint:
		if (NextIndexToAccess >= (**NextHandleToAccess).NumBytes)
			{
				NextHandleToAccess = (**NextHandleToAccess).Next;
				NextIndexToAccess = 0;
			}
		return;
	}


/* write data to the node currently being accessed */
MyBoolean		CSack::RewriteCurrent(void* PlaceToGet)
	{
		register short		Scan;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CSack::RewriteCurrent called on uninitialized object."));
		ERROR(!Scannable,PRERR(ForceAbort,
			"CSack::RewriteCurrent called on unscannable object."));
	 RestartPoint:
		if ((NextHandleToAccess == NIL) || (NextIndexToAccess < 0))
			{
				return False;
			}
		HRNGCHK(NextHandleToAccess,&((**NextHandleToAccess).Data[NextIndexToAccess]),
			BytesPerElement);
		MemCpy(&((**NextHandleToAccess).Data[NextIndexToAccess]),PlaceToGet,BytesPerElement);
		return True;
	}


MyBoolean		CSack::DoesItContain(void* Element)
	{
		Handle		DataTemp;

		DataTemp = AllocHandle(BytesPerElement);
		HLock(DataTemp);
		ResetScan();
		while (GetNext(*DataTemp))
			{
				if (MemEqu(*DataTemp,Element,BytesPerElement) == 0)
					{
						ReleaseHandle(DataTemp);
						return True;
					}
			}
		ReleaseHandle(DataTemp);
		return False;
	}
