/********************************************************************************
 *
 * PReadDDP.c		Written by Jon Hueras [76174,3267]
 *
 *	This file provides a capability analogous to DDPRead in the "alternate"
 *	AppleTalk interface for the "preferred" interface. You pass a pointer
 *	to the PReadDDPListener entry point below to POpenSkt. Subsequent calls
 *	to PReadDDP allow you to queue up one or more packet buffers to receive
 *	incoming packets to that socket. Another entry point provides the
 *	ability to flush all queued buffers for a particular socket.
 *
 *	The socket listener is designed so that you can assign it to multiple
 *	sockets. A separate queue is maintained for each possible socket number.
 *	This requires a 2K table containing 256 queue headers. If you cannot
 *	afford that much memory (?), it is fairly straightforward to modify the
 *	code to maintain a single queue, but then the resulting socket listener
 *	could only be attached to a single socket at a time.
 *
 * Interfaces:
 *
 *   void PReadDDP(Byte SocketNum, DDPPacket *PacketBuff, Boolean Async);
 *
 *	PReadDDP takes the PacketBuff given and places it at the end of the
 *	queue for the given SocketNum. The format of the PacketBuff is shown in
 *	PReadDDP.h and, except for the PacketReceived flag at the beginning,
 *	mirrors the packet format as received (and as described in Inside Mac
 *	Volume II). When a valid packet arrives on the given socket, the socket
 *	listener removes the PacketBuff from the socket's queue, places the
 *	packet data into the PacketBuff, and sets the flag (which is initially
 *	cleared by PReadDDP). If you pass FALSE for the Async parameter, PReadDDP
 *	will wait in an internal loop for the flag to be set before returning.
 *
 *	Note that if the LAP protocol type of the packet is shortDDP, then the
 *	CheckSum and the Net numbers will be cleared to zeros and the Node
 *	numbers will be copied from the LAP header. To return a packet to the
 *	sender, you need only copy the source Net, Node, and Socket fields to
 *	the corresponding destination fields and pass a pointer to the
 *	LAPDstNode field in your first WDS entry.
 *
 *	It is important to ensure that while a PacketBuff is queued it is not
 *	purged or moved or altered in any way. Similarly, as long as the socket
 *	listener is attached to any socket it must not be purged or moved. This
 *	means that any code segment or resource that contains the socket
 *	listener must be locked while it is in use.
 *
 *   void PReadDDPFlush(Byte SocketNum);
 *
 *	PReadDDPFlush will clear the socket listener's queue for the given
 *	socket number. This is not quite the same as DDPReadCancel, but purging
 *	individual queue elements doesn't make sense to me.
 *
 *	Because of the asynchronous nature of packet reception, one or more
 *	buffers that were queued may no longer be in the queue by the time the
 *	queue is flushed, i.e., they may have received packets. Buffers that are
 *	actually flushed from the queue will have their PacketReceived flags
 *	still clear, whereas buffers that have received packets will have their
 *	flags set.
 *
 *	If you close a socket that uses PReadDDPListener, you should flush the
 *	socket's queue unless you are certain it is already empty. This is in
 *	case you open another socket that gets the same socket number.
 *
 * Implementation Details:
 *
 *	The socket table, which is defined using DC.L directives has the
 *	following equivalent format in C:
 *
 *		struct {
 *		   DDPPacket *Head, *Tail;
 *		} SocketTable[256];
 *
 *	The Head field points to the first buffer in the queue and the Tail
 *	field points to the last buffer in the queue. If there is only one
 *	buffer in the queue, then both the Head and the Tail point to it. If the
 *	queue is empty, then both the Head and the Tail are NULL (zero).
 *
 *	While a packet buffer is in a queue, it is treated as though it had the
 *	following format:
 *
 *		struct {
 *		   Boolean   PacketReceived;
 *		   DDPPacket *NextBuffer;
 *		};
 *
 *	When the socket listener is called, it may not necessarily result in the
 *	incoming packet being received into a buffer. A packet may be rejected
 *	for one of three reasons:
 *
 *	   1. There are no buffers queued for the socket.
 *
 *	   2. There was a low-level error in the packet (such as a CRC
 *	      or framing error) reported by MPP.
 *
 *	   3. The packet was of the longDDP form, had a non-zero checksum,
 *	      and the computed checksum didn't match the received one.
 *
 ********************************************************************************/


/********************************************************************************
 *	The #define immediately preceding the #include below inhibits the
 *	prototype declarations in the #include. Alternate prototypes follow
 *	the #include. This is all made necessary by the fact that all three
 *	functions are actually entry points to a single assembly language
 *	function. The subsequent #defines provide symbolic references to the
 *	stack-based parameters of PReadDDP and PReadDDPFlush.
 ********************************************************************************/

#define _No_PReadDDP_Prototypes_
#include "PReadDDP.h"

void PReadDDP(void);
void PReadDDPFlush(void);
void PReadDDPListener(void);

#define SocketNum	 4(a7)
#define PacketBuff	 6(a7)
#define Async		10(a7)

/********************************************************************************
 *	Since neither AppleTalk.h nor nAppleTalk.h provide any support for
 *	socket listeners, the following #defines help to make things more
 *	readable. See the AppleTalk Manager chapter in IM Vol. II for details.
 ********************************************************************************/

#define ReadRest	2(a4)

#define toRHA		1
#define RHA_LapProto	toRHA+2(a2)
#define RHA_DDPLength	toRHA+3(a2)
#define RHA_DDPChecksum	toRHA+5(a2)
#define RHA_DDPDstNet	toRHA+7(a2)

#define shortDDP	1

/********************************************************************************
 *	PktOffset is a macro that is merely easier to type and read than using
 *	OFFSET(DDPPacket, ...). NextBuff is a macro that makes references to
 *	a buffer's queue link field easier to read. Defining a union or another
 *	struct would have accomplished this as well. The PacketReceived macro
 *	is purely cosmetic.
 ********************************************************************************/

#define PktOffset(field)	OFFSET(DDPPacket, field)
#define NextBuff(reg)		2(reg)
#define PacketReceived(reg)	(reg)


void PReadDDP(void)
  {
    asm {
	
	; Get the address of the socket table and the socket number. Use the socket
	; number to index into the socket table to the queue header for the socket.
	; Then get the packet buffer address.
	
		lea	@SocketTable, a1
		moveq	#0, d0
		move.b	SocketNum, d0
		lsl.w	#3, d0				;socket num * 8 bytes per table entry
		add.w	d0, a1
	    	move.l	PacketBuff, a0
	
	; Disable interrupts to prevent the socket listener from manipulating the
	; queue while we are in the middle of changing it. Interrupts will only be
	; disabled for a few microseconds.
		
		move	sr, d1
		ori	#0x0700, sr
	
	; See if the queue is currently empty
		
		tst.l	(a1)+
		beq.s	@QEmpty
	
	; If the queue isn't empty, we make both the last queue entry and the
	; queue header Tail point to the new buffer.
		
		move.l	(a1), d0
		move.l	a0, (a1)
		move.l	d0, a1
		move.l	a0, NextBuff(a1)
		bra.s	@ClearFlag
	
	; If the queue is empty, we just make both the Head and Tail of the queue
	; header point to it.
		
QEmpty:		move.l	a0, (a1)
		move.l	a0, -(a1)
	
	; Clear the PacketReceived flag and the queue link and reenable interrupts.
		
ClearFlag:	clr.b	PacketReceived(a0)
	    	clr.l	NextBuff(a0)
		move	d1, sr
	
	; Check the Async parameter. If FALSE, wait for the PacketReceived flag to
	; be set by the socket listener before returning. Otherwise return
	; immediately.
		
		tst.b	Async
		bne.s	@Asynch
		
Synch:		tst.b	PacketReceived(a0)
		beq.s	@Synch
		
Asynch:		rts


extern PReadDDPFlush:
	
	; Get the address of the socket table and the socket number. Use the socket
	; number to index into the socket table to the queue header for the socket.
	
		lea	@SocketTable, a1
		moveq	#0, d0
		move.b	SocketNum, d0
		lsl.w	#3, d0				;socket num * 8 bytes per table entry
		add.w	d0, a1
	
	; Disable interrupts to prevent the socket listener from manipulating the
	; queue while we are in the middle of changing it. Interrupts will only be
	; disabled for a couple of microseconds.
		
		move	sr, d1
		ori	#0x0700, sr
	
	; Clear the Head and Tail of the queue header.
	
		clr.l	(a1)+
		clr.l	(a1)
	
	; Reenable interrupts and return
	
		move	d1, sr
		rts


extern PReadDDPListener:

	; Upon entry to the socket listener, registers are set as follows:
	;
	;	A0-A1	Reserved for use by MPP
	;	A2	Pointer to MPP globals
	;	A3	Doesn't contain anything useful - needed to pass param to ReadRest
	;	A4	Points to ReadPacket and ReadRest jump vectors
	;	A5	Available for use until ReadRest is called, but we don't need it
	;
	;	D0	Destination socket number
	;	D1	Number of bytes remaining to be read in packet
	;	D2	Available for use
	;	D3	Available for use, but needed for parameter passing to ReadRest
	;
	; After ReadRest has been called A4 and A5 are no longer used or available.
	; A0-A3 are available for use, but A2 still contains a pointer to MPP globals
	; and we'll continue to use it to access MPP's Read Header Area (RHA), which
	; holds the packet's DDP header. D0-D3 are available for use.
	
	
	; Get the address of the socket table and the socket number. Use the socket
	; number to index into the socket table to the queue header for the socket.
	
		lea	@SocketTable, a3
		and.w	#0x00FF, d0
		lsl.w	#3, d0				;socket num * 8 bytes per table entry
		add.w	d0, a3
		move.l	a3, d2				;save address for later
	
	; Use the Head of the queue header to get the next available buffer. If the
	; queue is empty, discard the packet.
	
		move.l	(a3), d0
		move.l	d0, a3
		bne.s	@GotBuffer

		moveq	#0, d3				;flush packet and return
		jmp	ReadRest
	
	; Call ReadRest to pull the rest of the incoming packet into the Data portion
	; of the packet buffer. If ReadRest returns an error, discard the packet.

GotBuffer:	add.w	#PktOffset(Data), a3
		move.w	d1, d3
		jsr	ReadRest
		bne.s	@ExitListener
	
	; Retrieve the queue header and packet buffer addresses and check the LAP
	; protocol type in the RHA. We need to react differently depending on whether
	; the packet is shortDDP or longDDP.
	
		move.l	d2, a3
		move.l	(a3), a1
		cmp.b	#shortDDP, RHA_LapProto
		beq.s	@IsShortDDP
	
	; If the packet is longDDP and the Checksum (in the RHA) is non-zero, then
	; we need to compute the packet checksum and compare it with the received
	; value.
	
		tst.w	RHA_DDPChecksum			;if packet checksum zero
		beq.s	@NoChecksum			;no need to check
	
	; Checksum the header bytes in the RHA
	
		moveq	#0, d3
		lea	RHA_DDPDstNet, a0
		moveq	#9, d0
		bsr.s	@DoChecksum
	
	; Checksum the data bytes
		
		lea	PktOffset(Data)(a1), a0
		move.w	RHA_DDPLength, d0		;get packet length from RHA
		and.w	#0x03FF, d0			;mask out hop count &c.
		sub.w	#13, d0				;discount hdr length
		bsr.s	@DoChecksum
	
	; If the checksum happens to come out zero, we substitute -1 instead. We
	; then compare the computed and received checksum and if they don't match
	; we discard the packet.
		
		bne.s	@NonZChecksum
		subq.w	#1, d3
NonZChecksum:	cmp.w	RHA_DDPChecksum, d3
		bne.s	@ExitListener
	
	; Point the Head of the queue header to the next packet buffer. If there is
	; no next buffer, clear the Tail as well.

NoChecksum:	move.l	NextBuff(a1), (a3)+
		bne.s	@QNonEmpty
		clr.l	(a3)
	
	; Copy the packet header bytes from the RHA to the packet buffer.
	
QNonEmpty:	move.l	a1, a3				;save buffer addr
		lea	toRHA(a2), a0
		addq.w	#PktOffset(LAPDstNode), a1
		moveq	#16, d0
		bsr.s	@MoveBytes
	
	; Set the PacketReceived flag and return
	
		move.b	#1, PacketReceived(a3)
		rts
	
	; For shortDDP packets there is no checksumming, but the header format is
	; different and we want to arrange things in the packet buffer for the
	; convenience of the caller. We first detach the packet buffer from the
	; queue.

IsShortDDP:	move.l	NextBuff(a1), (a3)+		;put next buffer in queue head
		bne.s	@QIsNotEmpty
		clr.l	(a3)				;clear queue tail if queue empty
	
	; Copy the packet header from the RHA to the packet buffer up to and including
	; the packet length field.
	
QIsNotEmpty:	move.l	a1, a3				;save buffer addr
		lea	toRHA(a2), a0
		addq.w	#PktOffset(LAPDstNode), a1
		moveq	#5, d0
		bsr.s	@MoveBytes
	
	; Clear the Checksum and the DstNet and SrcNet fields in the packet buffer.
		
		clr.l	(a1)+
		clr.w	(a1)+
	
	; Copy the DstNode and SrcNode from the LAP header
	
		move.b	PktOffset(LAPDstNode)(a3), (a1)+
		move.b	PktOffset(LAPSrcNode)(a3), (a1)+
	
	; Copy the rest of the header (DstSocket, SrcSocket, DDPProtoType)
	
		moveq	#3, d0
		bsr.s	@MoveBytes
	
	; Set the PacketReceived flag and return
	
		move.b	#1, PacketReceived(a3)
ExitListener:	rts

	
	; DoChecksum
	;
	;	Computes the DDP checksum for a sequence of bytes. D3 is used to hold
	;	the running checksum and should be initialized to zero before the
	;	first call. A0 is used to pass a pointer to the bytes to be
	;	checksummed and D0 is used to pass the number of bytes. D1 is
	;	used internally.
	
DoChecksum:	subq.w	#1, d0				;decrement length for DBRA
		bmi.s	@CSExit				;exit if length was <= 0
		moveq	#0, d1				;clear upper bits of D1
CSLoop:		move.b	(a0)+, d1			;copy a byte into D1
		add.w	d1, d3				;add the *word* in D1 to the checksum
		rol.w	#1, d3				;rotate the checksum 1 bit left
		dbra	d0, @CSLoop			;loop if more
CSExit:		rts

	
	; MoveBytes
	;
	;	SImilar to BlockMove, but we don't want to use BlockMove because it
	;	is less efficient for a small number of bytes and may furthermore
	;	be patched, making it even slower. Parameter passing is the same,
	;	but the registers aren't saved and restored. A0 is the source pointer,
	;	A1 is the destination pointer, and D0 is the number of bytes to move.

MoveBytes:	subq.w	#1, d0				;decrement length for DBRA
		bmi.s	@MBExit				;exit if length was <= 0
MBLoop:		move.b	(a0)+, (a1)+
		dbra	d0, @MBLoop
MBExit:		rts

	
	; Here, at last, is the socket table. It contains 256 queue headers. Each
	; queue header contains two pointers of 4 bytes each.
	;
	; 	256*2*4 = 2048 bytes
	;	256*2   = 512 longs
	;
	; So you don't have to count, there are 32 longs per row below and 16 rows.
	;
	;	32*16   = 512 longs

SocketTable:
	dc.l	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	dc.l	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	dc.l	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	dc.l	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	dc.l	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	dc.l	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	dc.l	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	dc.l	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	dc.l	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	dc.l	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	dc.l	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	dc.l	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	dc.l	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	dc.l	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	dc.l	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	dc.l	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    }
  }