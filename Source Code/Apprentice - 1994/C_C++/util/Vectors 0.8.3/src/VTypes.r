type 'MRPd' {
	integer;						/* DLOG ID */
	unsigned bitstring[4] = 0;
	boolean lrfc,fclr;
	boolean nonotes,remarksarenotes;
	boolean noremarks,remarksfield;
	boolean straight,staggered;
	fill byte;
	byte;							/* nSta */
	byte = $$Countof(ColArray);
	wide array ColArray {
		boolean nosplit,split;
		boolean wide,narrow;
		bitstring[4] = 0;
		bitstring[2]
						tabright = 0,
						tabdown = 1,
						tabback = 2;
		byte
						blank = 0,
						fromstation = 1,
						tostation = 2,
						station = 17,
						distance = 3,
						foreazimuth = 4,
						backazimuth = 5,
						azimuth = 20,
						foreclinometer = 6,
						backclinometer = 7,
						clinometer = 22,
						instrumentheight = 8,
						targetheight = 9,
						height = 24,
						fromdimensions = 10,
						todimensions = 11,
						dimensions = 26,
						remarks = 12;
	};
};
