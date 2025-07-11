/* CArray.c */

#include "CArray.h"
#include "Memory.h"


/* */				CArray::CArray()
	{
		ArrayType**		Temp;

		Temp = (ArrayType**)AllocHandle(sizeof(ArrayType));
		FirstBlock = Temp;
		(**Temp).Next = NIL;
		(**Temp).NumElements = 0;
	}


/* */				CArray::~CArray()
	{
		ArrayType**		Scan;

		while (FirstBlock != NIL)
			{
				Scan = FirstBlock;
				FirstBlock = (**FirstBlock).Next;
				ReleaseHandle((Handle)Scan);
			}
	}


void				CArray::IArray(long TheBytesPerElement, long TheElementsPerArray)
	{
		BytesPerElement = TheBytesPerElement;
		ElementsPerArray = TheElementsPerArray;
		EXECUTE(ElementsPerArray = 4;)
	}


long				CArray::GetNumElements(void)
	{
		long					Count;
		ArrayType**		Scan;

		Count = 0;
		Scan = FirstBlock;
		while (Scan != NIL)
			{
				Count += (**Scan).NumElements;
				Scan = (**Scan).Next;
			}
		return Count;
	}


long				CArray::AppendElement(void)
	{
		ArrayType**		Scan;
		ArrayType**		Temp;
		ArrayType**		Lag;
		ArrayType**		Lag2;
		long					Index;

		Scan = FirstBlock;
		Index = 0;
		Lag = NIL;
		Lag2 = NIL;
		do
			{
				Index += (**Scan).NumElements;
				Lag2 = Lag;
				Lag = Scan;
				Scan = (**Scan).Next;
			} while (Scan != NIL);
		if ((**Lag).NumElements >= ElementsPerArray)
			{
				/* adding whole new block on the end */
				Temp = (ArrayType**)AllocHandle(sizeof(ArrayType) + BytesPerElement);
				(**Temp).Next = NIL;
				(**Temp).NumElements = 1;
				(**Lag).Next = Temp;
				return Index;
			}
		 else
			{
				/* appending to end of last block */
				Temp = (ArrayType**)AllocHandle(HandleSize((Handle)Lag) + BytesPerElement);
				(**Temp).Next = (**Lag).Next;
				(**Temp).NumElements = (**Lag).NumElements + 1;
				HRNGCHK(Temp,&((**Temp).Data),BytesPerElement * (**Lag).NumElements);
				MemCpy((**Temp).Data,(**Lag).Data,BytesPerElement * (**Lag).NumElements);
				ReleaseHandle((Handle)Lag);
				if (Lag2 == NIL)
					{
						FirstBlock = Temp;
					}
				 else
					{
						(**Lag2).Next = Temp;
					}
				return Index;
			}
	}


long				CArray::InsertElement(long IndexToInsertBefore)
	{
		long				IndexOfFirst;
		long				IndexAfterLast;
		ArrayType**	Scan;
		ArrayType**	Lag;
		ArrayType**	Temp;

	 RetryPoint:
		Scan = FirstBlock;
		IndexOfFirst = 0;
		Lag = NIL;
	 RepeatPoint:
		IndexAfterLast = IndexOfFirst + (**Scan).NumElements;
		if ((IndexToInsertBefore >= IndexOfFirst)
			&& (IndexToInsertBefore < IndexAfterLast))
			{
				if ((**Scan).NumElements >= ElementsPerArray)
					{
						ArrayType**		FirstHalf;
						ArrayType**		SecondHalf;
						long					ElementsInFirstHalf;
						long					ElementsInSecondHalf;
						long					nScan;

						ElementsInFirstHalf = (**Scan).NumElements / 2;
						ElementsInSecondHalf = (**Scan).NumElements - ElementsInFirstHalf;
						FirstHalf = (ArrayType**)AllocHandle(sizeof(ArrayType)
							+ (ElementsInFirstHalf * BytesPerElement));
						SecondHalf = (ArrayType**)AllocHandle(sizeof(ArrayType)
							+ (ElementsInSecondHalf * BytesPerElement));
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
						(**SecondHalf).Next = (**Scan).Next;
						(**SecondHalf).NumElements = ElementsInSecondHalf;
						HRNGCHK(FirstHalf,&((**FirstHalf).Data[0]),
							BytesPerElement * (**FirstHalf).NumElements);
						HRNGCHK(Scan,&((**Scan).Data[0]),
							BytesPerElement * (**FirstHalf).NumElements);
						MemCpy(&((**FirstHalf).Data[0]),&((**Scan).Data[0]),
							BytesPerElement * (**FirstHalf).NumElements);
						HRNGCHK(SecondHalf,&((**SecondHalf).Data[0]),
							BytesPerElement * (**SecondHalf).NumElements);
						HRNGCHK(Scan,&((**Scan).Data[
							BytesPerElement * (**FirstHalf).NumElements]),
							BytesPerElement * (**SecondHalf).NumElements);
						MemCpy(&((**SecondHalf).Data[0]),&((**Scan).Data[
							BytesPerElement * (**FirstHalf).NumElements]),
							BytesPerElement * (**SecondHalf).NumElements);
						ReleaseHandle((Handle)Scan);
						/* go back and make sure we are still inserting into proper block */
						goto RetryPoint;
					}
				 else
					{
						long				Reference;

						/* opening hole in block */
						/* Now we have the block (Scan) so where to insert into it? */
						Reference = IndexToInsertBefore - IndexOfFirst;
						/* insert into relative position of <Reference>. */
						Temp = (ArrayType**)AllocHandle(HandleSize((Handle)Scan) + BytesPerElement);
						(**Temp).Next = (**Scan).Next;
						(**Temp).NumElements = (**Scan).NumElements + 1;
						if (Lag == NIL)
							{
								FirstBlock = Temp;
							}
						 else
							{
								(**Lag).Next = Temp;
							}
						/* copying over preceding zone */
						HRNGCHK(Temp,&((**Temp).Data[0]),Reference * BytesPerElement);
						HRNGCHK(Scan,&((**Scan).Data[0]),Reference * BytesPerElement);
						MemCpy(&((**Temp).Data[0]),&((**Scan).Data[0]),Reference * BytesPerElement);
						/* copying over element (there isn't one so nothing really happens.) */
						HRNGCHK(Temp,&((**Temp).Data[Reference * BytesPerElement]),BytesPerElement);
						/* MemCpy(&((**Temp).Data[Reference * BytesPerElement]),
							Element,BytesPerElement); */
						/* copying over succeeding range */
						HRNGCHK(Temp,&((**Temp).Data[(Reference * BytesPerElement)
							+ BytesPerElement]),(BytesPerElement * (**Scan).NumElements)
							- (Reference * BytesPerElement));
						HRNGCHK(Scan,&((**Scan).Data[Reference * BytesPerElement]),
							(BytesPerElement * (**Scan).NumElements) - (Reference * BytesPerElement));
						MemCpy(&((**Temp).Data[(Reference * BytesPerElement) + BytesPerElement]),
							&((**Scan).Data[Reference * BytesPerElement]),
							(BytesPerElement * (**Scan).NumElements) - (Reference * BytesPerElement));
						ReleaseHandle((Handle)Scan);
					}
				return IndexToInsertBefore;
			}
		IndexOfFirst = IndexAfterLast;
		if ((**Scan).Next == NIL)
			{
				/* oops, we ran off end.  call AppendElement */
				return AppendElement();
			}
		Lag = Scan;
		Scan = (**Scan).Next;
		goto RepeatPoint;
	}


MyBoolean		CArray::DeleteElement(long TheOneToDelete)
	{
		long				IndexOfFirst;
		long				IndexAfterLast;
		long				IndexToDelete;
		ArrayType**	Scan;
		ArrayType**	Lag;
		ArrayType**	Temp;

	 RetryPoint:
		Scan = FirstBlock;
		IndexOfFirst = 0;
		Lag = NIL;
	 RepeatPoint:
		IndexAfterLast = IndexOfFirst + (**Scan).NumElements;
		if ((TheOneToDelete >= IndexOfFirst)
			&& (TheOneToDelete < IndexAfterLast))
			{
				IndexToDelete = TheOneToDelete - IndexOfFirst;
				Temp = (ArrayType**)AllocHandle(((**Scan).NumElements - 1) * BytesPerElement
					+ sizeof(ArrayType));
				(**Temp).Next = (**Scan).Next;
				(**Temp).NumElements = (**Scan).NumElements - 1;
				HRNGCHK(Temp,&((**Temp).Data[0]),IndexToDelete * BytesPerElement);
				HRNGCHK(Scan,&((**Scan).Data[0]),IndexToDelete * BytesPerElement);
				MemCpy(&((**Temp).Data[0]),&((**Scan).Data[0]),IndexToDelete * BytesPerElement);
				HRNGCHK(Temp,&((**Temp).Data[IndexToDelete * BytesPerElement]),
					((**Scan).NumElements - 1) * BytesPerElement - IndexToDelete * BytesPerElement);
				HRNGCHK(Scan,&((**Scan).Data[IndexToDelete * BytesPerElement + BytesPerElement]),
					((**Scan).NumElements - 1) * BytesPerElement - IndexToDelete * BytesPerElement);
				MemCpy(&((**Temp).Data[IndexToDelete * BytesPerElement]),
					&((**Scan).Data[IndexToDelete * BytesPerElement
					+ BytesPerElement]),((**Scan).NumElements - 1) * BytesPerElement
					- IndexToDelete * BytesPerElement);
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
				return True;
			}
		IndexOfFirst = IndexAfterLast;
		if ((**Scan).Next == NIL)
			{
				return False;
			}
		Lag = Scan;
		Scan = (**Scan).Next;
		goto RepeatPoint;
	}


MyBoolean		CArray::GetElement(long Index, void* PlaceToPut)
	{
		long				IndexOfFirst;
		long				IndexAfterLast;
		ArrayType**	Scan;
		ArrayType**	Lag;
		ArrayType**	Temp;

		StackSizeTest();
		Scan = FirstBlock;
		IndexOfFirst = 0;
		Lag = NIL;
	 RepeatPoint:
		IndexAfterLast = IndexOfFirst + (**Scan).NumElements;
		if ((Index >= IndexOfFirst)
			&& (Index < IndexAfterLast))
			{
				MemCpy(PlaceToPut,&((**Scan).Data[(Index - IndexOfFirst) * BytesPerElement]),
					BytesPerElement);
				return True;
			}
		IndexOfFirst = IndexAfterLast;
		if ((**Scan).Next == NIL)
			{
				return False;
			}
		Lag = Scan;
		Scan = (**Scan).Next;
		goto RepeatPoint;
	}


MyBoolean		CArray::PutElement(long Index, void* PlaceToGet)
	{
		long				IndexOfFirst;
		long				IndexAfterLast;
		ArrayType**	Scan;
		ArrayType**	Lag;
		ArrayType**	Temp;

		Scan = FirstBlock;
		IndexOfFirst = 0;
		Lag = NIL;
	 RepeatPoint:
		IndexAfterLast = IndexOfFirst + (**Scan).NumElements;
		if ((Index >= IndexOfFirst)
			&& (Index < IndexAfterLast))
			{
				MemCpy(&((**Scan).Data[(Index - IndexOfFirst) * BytesPerElement]),
					PlaceToGet,BytesPerElement);
				return True;
			}
		IndexOfFirst = IndexAfterLast;
		if ((**Scan).Next == NIL)
			{
				return False;
			}
		Lag = Scan;
		Scan = (**Scan).Next;
		goto RepeatPoint;
	}


void*				CArray::GetElementAddress(long Index)
	{
		long				IndexOfFirst;
		long				IndexAfterLast;
		ArrayType**	Scan;
		ArrayType**	Lag;
		ArrayType**	Temp;

		Scan = FirstBlock;
		IndexOfFirst = 0;
		Lag = NIL;
	 RepeatPoint:
		IndexAfterLast = IndexOfFirst + (**Scan).NumElements;
		if ((Index >= IndexOfFirst)
			&& (Index < IndexAfterLast))
			{
				return &((**Scan).Data[(Index - IndexOfFirst) * BytesPerElement]);
			}
		IndexOfFirst = IndexAfterLast;
		if ((**Scan).Next == NIL)
			{
				return NIL;
			}
		Lag = Scan;
		Scan = (**Scan).Next;
		goto RepeatPoint;
	}


MyBoolean		CArray::KillElement(void* Element)
	{
		long			NumThings;
		long			Scan;
		void*			Address;

		NumThings = GetNumElements();
		Scan = 0;
		while (Scan < NumThings)
			{
				Address = GetElementAddress(Scan);
				if (MemEqu(Element,Address,BytesPerElement))
					{
						DeleteElement(Scan);
						return True;
					}
				Scan += 1;
			}
		return False;
	}
