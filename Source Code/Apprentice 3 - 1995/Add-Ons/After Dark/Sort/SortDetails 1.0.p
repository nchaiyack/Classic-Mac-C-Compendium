unit SortDetails;
{Written and © by Scott Lindhurst, lindhurs@math.wisc.edu, fall 1993 and Nov. 1994.}
{Mail:	1107 Erin St.		Madison, WI 53715-1845}
{or	123 Millwood Dr.		Tonawanda, NY 14150-5513}
{If you use this source in your program, give me credit in the about box and documentation.}


{Holds the dirty details of the sorts.}
{Sources: Sedgewick’s Algorithms book and Knuth volume 3.}

{Possible additions: make Bubble sort, Shaker sort, Insertion sort into subclasses of one}
{insertion-type-sort class. This may also include Shell sort.}

interface
	uses
		PixelUtils, ObjIntf;

	type
		cSorter = object(TObject)
				Done: boolean;		{The procedure that finishes the sort should set this to true}
				ScreenSize: longint;	{The last position to sort. Saved by Init procedure.}
				procedure cSorter.Init (N: longint);
{Prepare to sort pixels from 1 to N, inclusive.}
				procedure cSorter.DoALittle;
			end;

		cBatcherSort = object(cSorter)
				p, q, r, d, savePower, left: longint;
				InitPLoop: boolean;
				procedure cBatcherSort.Init (N: longint);
				override;
				procedure cBatcherSort.DoALittle;
				override;
			end;

		cBubbleSort = object(cSorter)
				Bound, left, lastMoved: longint;
				procedure cBubbleSort.Init (N: longint);
				override;
				procedure cBubbleSort.DoALittle;
				override;
			end;

		cShakerSort = object(cSorter)
				TopBound, BotBound, left, right, lastMoved: longint;
				goingLeft: boolean;	{true if this pass is going left, false if it’s going right.}
				procedure cShakerSort.Init (N: longint);
				override;
				procedure cShakerSort.DoALittle;
				override;
			end;

		cShellSort = object(cSorter)
				left, stepSize: longint;
				stepSizeSorted: boolean;	{true if every subfile {i, i+stepSize, i+2*stepSize,… is sorted}
				procedure cShellSort.Init (N: longint);
				override;
				procedure cShellSort.DoALittle;
				override;
			end;

		cHeapSort = object(cSorter)
				heapMade: boolean;		{true if the screen has the heap property.}
				heapSize: longint;		{Size of heap}
				NextToDo: longint;		{Next position to downHeap or sort}
				procedure cHeapSort.Init (N: longint);
				override;
				procedure cHeapSort.DoALittle;
				override;
				procedure cHeapSort.DownHeap (downNode: longint);
			{Move the node down the heap. Depends on heapSize being right.}
			end;

		cSmallInsertion = object(cSorter)
{Only gives good multitasking when the screen is almost sorted, i.e. no insertion step}
{takes too long. Designed for use with a QuickSort that ignores small subfiles, like the one below.}
				left: longint;
				procedure cSmallInsertion.Init (N: longint);
				override;
				procedure cSmallInsertion.DoALittle;
				override;
			end;

		cQuicksort = object(cSorter)
{A quicksort with recursion removed, median-of-three partitioning, and ignoring small}
{subfiles until the end. It should be pretty fast.}
				doingInsertion: boolean;		{These two are used for the insertion sort of small}
				insSorter: cSmallInsertion;	{subfiles at the end.}
				stack: array[0..64] of longint;	{the price we pay for removing recursion}
				StackTop: longint;		{top of the stack}
				doingPartition: boolean;	{true if in the middle of a partitioning operation.}
				ptnLeft, ptnRight, ptnValue: longint;		{left and right pointers if partitioning.}
				procedure cQuicksort.Init (N: longint);
				override;
				function cQuickSort.Partition: longint;
{Using data in SELF, continue the partition or start it.}
				procedure cQuicksort.DoALittle;
				override;
			end;

implementation

	const
		kSliceOps = 1000;	{Aim to do about this many (or fewer) Get or SetPixel per call.}
{SwapPixels counts as 4 ops: 2 Get and 2 Set.}
{SortPair counts as 3 ops: 2 Get, and either 0 or 2 Set.}
		kQSPartitionMin = 10;	{Don’t bother quicksorting anything this short; leave for insertion.}

	procedure cSorter.Init (N: longint);
	begin
		Done := false;
		ScreenSize := N;
	end;

	procedure cSorter.DoALittle;
	begin
	end;

	procedure cBatcherSort.Init (N: longint);
	begin
		p := 1;
		while (p < N) do
			p := 2 * p;
		p := p div 2;		{Now p is the highest power of 2 which is < N}
		savePower := p;
		left := 0;
		InitPLoop := true;	{Yes, we need to start the loop on p.}
		inherited Init(N);
	end;

	procedure cBatcherSort.DoALittle;
{Batcher’s merge-exchange method; from Knuth vol. 3 p.112}
		var
			i, right: longint;
	begin
		if InitPLoop then
			begin	{Initialize the loop on p}
				q := savePower;
				r := 0;
				d := p;
				InitPLoop := false;
			end;
		right := left + kSliceOps div 3;
		if right > ScreenSize - d then
			right := ScreenSize - d;
		for i := left to right do
			if BAnd(i, p) = r then
				SortPair(i + 1, i + d + 1);
		left := right + 1;
		if right = ScreenSize - d then
			begin	{the loop i:=0 to ScreenSize - d just finished}
				left := 0;
				d := q - p;
				q := q div 2;
				r := p;
			end;
		if d = 0 then
			begin	{The loop on d just ended; continue the loop on p next time, unless we’re done.}
				p := p div 2;
				InitPLoop := true;
				if p = 0 then
					Done := true;
			end;
	end;	{Batcher’s sort}



	procedure cShakerSort.Init (N: longint);
	begin
		TopBound := N;	{everything to the right of here is correct}
		BotBound := 1;	{everything to the left of here is correct.}
		left := 1;
		right := N;
		goingLeft := false;
		lastMoved := 0;
		inherited init(N);
	end;	{procedure cShakerSort.Init}

	procedure cShakerSort.DoALittle;
		var
			lastMoved, j: longint;
			pix1, pix2: pixelRec;
	begin
		if goingLeft then
			begin
				left := right - (kSliceOps div 3);
				if left <= BotBound + 1 then
					left := BotBound + 1;
				for j := right downto left do
					begin
						pix1 := MyGetPixel(j - 1);
						pix2 := MyGetPixel(j);
						if pix1.sortValue > pix2.SortValue then
							begin
								MySetPixel(j - 1, pix2);
								MySetPixel(j, pix1);
								lastMoved := j;
							end;
					end;	{for}
				right := left - 1;	{prepare for next time}
				if left = BotBound + 1 then	{Above loop really terminated}
					if lastMoved = 0 then
						Done := true
					else		{Not done; we’ll have to loop again.}
						begin
							BotBound := lastMoved;	{stuff to the left of here is sorted.}
							goingLeft := false;
							left := BotBound;
							lastMoved := 0;
						end;
			end	{going left}
		else	{going right}
			begin
				right := left + (kSliceOps div 3);
				if right >= TopBound - 1 then
					right := TopBound - 1;
				for j := left to right do
					begin
						pix1 := MyGetPixel(j);
						pix2 := MyGetPixel(j + 1);
						if pix1.sortValue > pix2.SortValue then
							begin
								MySetPixel(j, pix2);
								MySetPixel(j + 1, pix1);
								lastMoved := j;
							end;
					end;	{for}
				left := right + 1;	{prepare for next time}
				if right = TopBound - 1 then	{Above loop really terminated}
					if lastMoved = 0 then
						self.Done := true
					else		{Not done; we’ll have to loop again.}
						begin
							TopBound := lastMoved;	{stuff to the right of here is sorted.}
							goingLeft := true;
							right := TopBound;
							lastMoved := 0;
						end;
			end	{going left}
	end;	{procedure cShakerSort.DoALittle}

	procedure cBubbleSort.Init (N: longint);
	begin
		Bound := N;
		left := 1;
		lastMoved := 0;
		inherited init(N);
	end;

	procedure cBubbleSort.DoALittle;
		var
			lastMoved, right, j: longint;
			pix1, pix2: pixelRec;
	begin
		right := left + (kSliceOps div 3);
		if right >= Bound then
			right := Bound;
		for j := left to right do
			begin
				pix1 := MyGetPixel(j);
				pix2 := MyGetPixel(j + 1);
				if pix1.sortValue > pix2.SortValue then
					begin
						MySetPixel(j, pix2);
						MySetPixel(j + 1, pix1);
						lastMoved := j;
					end;
			end;	{for}
		left := right + 1;	{prepare for next time}
		if right = Bound then	{Above loop really terminated}
			if lastMoved = 0 then
				self.Done := true
			else		{Not done; we’ll have to loop again.}
				begin
					left := 1;
					Bound := lastMoved - 1;	{everything to the right of here is sorted.}
					lastMoved := 0;
				end;
	end;	{procedure cBubbleSort.DoALittle}


	procedure cShellSort.Init (N: longint);
	begin
		stepSize := 1;
		repeat
			stepSize := 3 * stepSize + 1;
		until stepSize > N;
		stepSizeSorted := true;
		inherited Init(N);
	end;

	procedure cShellSort.DoALittle;
		var
			i, j, right: longint;
			savePixel, localPixel: pixelRec;
	begin
		if stepSizeSorted then
			begin
				stepSize := stepSize div 3;
				left := stepSize + 1;
				stepSizeSorted := false;
			end;
		if not stepSizeSorted then
			begin
				right := left + (kSliceOps div 5);
				if right >= ScreenSize then
					begin
						right := ScreenSize;
						stepSizeSorted := true;
					end;
				for i := left to right do
					begin
						savePixel := MyGetPixel(i);
						j := i;
						localPixel := MyGetPixel(j - stepSize);
						while localPixel.sortValue > savePixel.sortValue do
							begin
								MySetPixel(j, localPixel);
								j := j - stepSize;
								if j <= stepSize then
									leave;
								localPixel := MyGetPixel(j - stepSize);
							end;
						MySetPixel(j, savePixel);
					end;	{for}
				left := right + 1;	{update left end to pick up where we left off.}
			end;	{if not stepSizeSorted}

		if stepSizeSorted and (stepSize = 1) then
			Done := true;
	end;	{procedure cShellSort.DoALittle}





	procedure cHeapSort.Init (N: longint);
	begin
		heapMade := false;
		heapSize := N;
		nextToDo := heapSize div 2;
		inherited Init(N);
	end;	{procedure cHeapSort.Init}


	procedure cHeapSort.DoALittle;
		var
			topToDo, botToDo, k: longint;
	begin
		if not heapMade then
			begin	{downHeap a few things to make it more heaplike.}
				topToDo := self.nextToDo;
				botToDo := topToDo - (kSliceOps div 50);
				self.NextToDo := botToDo - 1;
				if botToDo <= 1 then
					begin
						botToDo := 1;
						heapMade := true;		{It will be true when the procedure ends.}
						nextToDo := heapSize;
					end;
				for k := topToDo downto botToDo do
					self.DownHeap(k);
			end	{Making a heap}
		else
	{Pull stuff out of the heap and turn it into sorted material.}
			for k := 1 to kSliceOps div 50 do
				begin
					SwapPixels(1, heapSize);
					heapSize := heapSize - 1;
					self.DownHeap(1);
					if heapSize <= 1 then
						begin
							self.Done := true;
							leave;	{the for loop}
						end;
				end;	{pulling out and sorting the heap}
	end;		{procedure cHeapSort.DoALittle}


	procedure cHeapSort.DownHeap (downNode: longint);
{Move the node down the heap.}
		var
			i, j: longint;
			savePixel, pixelJ, pixelJPlus: pixelRec;
	begin
		savePixel := MyGetPixel(downNode);
		while downNode <= (heapSize div 2) do
			begin
				j := downNode + downNode;
				pixelJ := MyGetPixel(j);
				if j < heapSize then
					begin
						pixelJPlus := MyGetPixel(j + 1);
						if pixelJ.sortValue < pixelJPlus.sortValue then
							begin
								pixelJ := pixelJPlus;
								j := j + 1;
							end;
					end;
				if savePixel.sortValue >= pixelJ.sortValue then
					leave;	{done}
				MySetPixel(downNode, pixelJ);
				downNode := j;
			end;
		MySetPixel(downNode, savePixel);
	end;	{procedure cHeapSort.DownHeap}



	procedure cSmallInsertion.Init (N: longint);
	begin
		left := 2;
		inherited Init(N);
	end;	{procedure cSmallInsertion.Init}

	procedure cSmallInsertion.DoALittle;
{Designed for use on an almost sorted file and called only from QuickSort.}
		var
			i, j, right: longint;
			value, pixelJLeft: PixelRec;
	begin
		right := self.left + (kSliceOps div 5);
		if right >= ScreenSize then
			begin
				right := ScreenSize;
				Done := true;
			end;
		for i := left to right do
			begin
				value := MyGetPixel(i);
				j := i;
				pixelJLeft := MyGetPixel(j - 1);
				while pixelJLeft.SortValue > value.SortValue do
					begin
						MySetPixel(j, pixelJLeft);
						j := j - 1;
						pixelJLeft := MyGetPixel(j - 1);
					end;
				MySetPixel(j, value);
			end;

		left := right + 1;	{Update left end to continue next time}
	end;	{procedure cSmallInsertion.DoALittle}



	procedure cQuickSort.Init (N: longint);
		var
			ignore: longint;
	begin
		stack[0] := 1;
		stack[1] := N;
		StackTop := 1;
		doingPartition := false;
		doingInsertion := false;
		New(insSorter);	{for the insertion sort at the end.}
		insSorter.Init(N);
		inherited Init(N);
	end;


	function cQuickSort.Partition: longint;
{Using data in SELF, continue the partition or start it.}
{The top 2 entries on the stack (Stack array) give the left and right ends of the current partitioning}
{process; the values of ptnLeft and ptnRight are where we are in the partition.}
{If doingPartition = false then initialize (quickly) to do a partition.}
{If doingPartition = true then do some partitioning; if it finishes, then  doingPartition will be}
{	set false and the partition element position will be returned.}
{Note that the return value is defined only if doingPartition is false.}
		var
			left, right, middle: longint;	{for the middle element used in median–of–three partitioning.}
			StopDiff: longint;	{Used to decide when to end a time slice.}
			pixLeft, pixRight: PixelRec;
	begin
		if not doingPartition then
			begin	{Prepare for a new partitioning process. The next call to this will actually do it.}
				self.doingPartition := true;
				left := self.Stack[StackTop - 1];
				right := self.Stack[StackTop];
				middle := (left + right) div 2;
				SortThree(left, middle, right);
				SwapPixels(left + 1, middle);
{Now L <= L+1 <= R  as pixel sort values.}
				self.ptnLeft := left + 1;
				self.ptnRight := right;
				self.ptnValue := MyGetPixel(self.ptnLeft).sortValue;
			end	{preparing to partition}
		else
			begin	{do some partitioning for real!}
{Stop when we have ptnLeft and ptnRight have together moved a distance of kSliceOps}
{i.e. (ptnLeft - origL) + (origR - ptnRight) >= kSliceOps, or}
{origR - origL - kSliceOps >= ptnRight - ptnLeft.}

				StopDiff := ptnRight - ptnLeft - (kSliceOps div 2);
				repeat
					repeat
						ptnLeft := ptnLeft + 1;
						pixLeft := MyGetPixel(ptnLeft);
					until pixLeft.sortValue >= ptnValue;
					repeat
						ptnRight := ptnRight - 1;
						pixRight := MyGetPixel(ptnRight);
					until pixRight.sortValue <= ptnValue;
					if ptnRight <= ptnLeft then
						begin	{finished the partitioning!}
							SwapPixels(Stack[StackTop - 1] + 1, ptnRight);	{Put partition element in place.}
							leave;
						end;
					MySetPixel(ptnLeft, pixRight);	{Swap the pixels.}
					MySetPixel(ptnRight, pixLeft);
					if StopDiff >= ptnRight - ptnLeft then
						leave;
				until false;	{the only way to leave is with one of the leaves above.}
{Partitioning is done if ptnRight <= ptnLeft, otherwise it’s just suspended.}
				if ptnRight <= ptnLeft then	{exited above because done}
					begin	{this is the only case in which a return value is defined.}
						doingPartition := false;
						Partition := ptnRight;
					end;
			end;	{Doing some real partitioning.}
	end;	{function cQuickSort.Partition}



	procedure cQuickSort.DoALittle;
{A quicksort with recursion removed. It assumes that there are <= 2^32 pixels to sort.}
{(Otherwise my stack will overflow}

		procedure StackPush (L, R: longint);
{Push L and R onto my stack. Note that other areas depend on this implementation of the}
{stack, so don’t think this is data structure abstraction. It’s only convience.}
		begin
			Stack[StackTop + 1] := L;
			Stack[StackTop + 2] := R;
			StackTop := StackTop + 2;
		end;	{procedure StackPush}

		var
			ignore, left, right: longint;	{Left and right endpoints of the current stage.}
			partitionPos: longint;	{Returned by partition, this element is in the right place.}
	begin
		if doingInsertion then
			begin	{Insertion part}
				insSorter.DoALittle;
				if insSorter.Done then
					self.Done := true;
			end	{Insertion part}
		else
			begin	{Quicksort part}
				if not doingPartition then	{Start partitioning the interval on top of the stack.}
					ignore := self.Partition
				else
					begin
						partitionPos := self.Partition;	{do some partitioning.}
						if not doingPartition then	{the partition just finished. Prepare for the next one.}
							begin	{We’ve finished some partitioning; prepare for the next pieces.}
								left := Stack[StackTop - 1];	{pop off old left and right values.}
								right := Stack[StackTop];
								StackTop := StackTop - 2;
								if (partitionPos - left) > (right - partitionPos) then	{If right half smaller, do it first.}
									if (right - partitionPos) > kQSPartitionMin then	{As long as it’s not too small.}
										begin
											StackPush(left, partitionPos - 1);	{Do left later,}
											StackPush(partitionPos + 1, right);	{right piece next.}
										end	{left half > right half > Min}
									else	{right half is small so leave it for later.}
										if (partitionPos - left) > kQSPartitionMin then	{if left half big,}
											StackPush(left, partitionPos - 1)					{push it so it gets done.}
										else	{left and right both small, so ignore them both.}
								else	{left half is smaller, not the right half.}
									if (left - partitionPos) > kQSPartitionMin then	{if left half not too small}
										begin
											StackPush(partitionPos + 1, right);	{push right half to do later}
											StackPush(left, partitionPos - 1);	{push left half to do now}
										end
									else	{left half small}
										if (right - partitionPos) > kQSPartitionMin then	{If right half is big…}
											StackPush(partitionPos + 1, right);					{…push it to do next.}
							end;	{if not doingPartition}
					end;
				if StackTop < 0 then	{Done with the quicksort part; start the insertion part now.}
					doingInsertion := true
			end;	{Quicksort part}
	end;	{Procedure cQuickSort.DoALittle}
end.