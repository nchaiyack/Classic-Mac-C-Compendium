unit Transformations;

interface

{$IFC UseFixedMath = TRUE }
	uses
		FixedMath, Matrix;

	type

		Matrix4 = array[1..4, 1..4] of fixed;
		Vector4 = array[1..4] of fixed;

{$ELSEC}
	uses
		Matrix;

{$ENDC}



(* Scale : modify transformation-Matrix T so that scaling along x,y,z is according to sx,sy,sz *)

	procedure MScale (var T: Matrix4; sx, sy, sz: real);


(* Translate : modify tranform-Matrix T so that translation along x,y,z is according to dx,dy,dz *)


	procedure MTranslate (var T: Matrix4; dx, dy, dz: real);

(* RotX : modify tranform-Matrix T so that  it will rotate around x-achsis according to phi *)

	procedure RotX (var T: Matrix4; phi: real);


(* RotX : modify tranform-Matrix T so that  it will rotate around y-achsis according to phi *)

	procedure RotY (var T: Matrix4; phi: real);


(* RotX : modify tranform-Matrix T so that  it will rotate around z-achsis according to phi *)

	procedure RotZ (var T: Matrix4; phi: real);

(* RotArbitraryAchsis : ROTATION AROUND ARBITRARY ACHSIS : *)
(* p1,p2 are two Points on the Rotation-achsis (together, p1and p2 form a 3D-Line), phi is the angle *)

	procedure RotArbitraryAchsis (var T: Matrix4; p, x: Vector4; phi: real);


(* ==================== *)

implementation

{$IFC UseFixedMath = FALSE }
(* Scale : modify transformation-Matrix T so that scaling along x,y,z is according to sx,sy,sz *)

	procedure MScale (var T: Matrix4; sx, sy, sz: real);

		var
			s: Matrix4;

	begin
		s := Identity;
		s.M[1, 1] := sx;
		s.M[2, 2] := sy;
		s.M[3, 3] := sz;
		s.IdentityFlag := FALSE;
		T := MMult(T, s);
	end;


(* Translate : modify tranform-Matrix T so that translation along x,y,z is according to dx,dy,dz *)


	procedure MTranslate (var T: Matrix4; dx, dy, dz: real);

		var
			tl: Matrix4;

	begin
		tl := Identity;
		tl.M[4, 1] := dx;
		tl.M[4, 2] := dy;
		tl.M[4, 3] := dz;
		tl.IdentityFlag := FALSE;
		T := MMult(T, tl);
	end;


(* RotX : modify tranform-Matrix T so that  it will rotate around x-achsis according to phi *)

	procedure RotX (var T: Matrix4; phi: real);

		var
			co, si: real;
			r: Matrix4;

	begin
		co := cos(phi);
		si := sin(phi);
		r := Identity;
		r.M[2, 2] := co;
		r.M[2, 3] := si;
		r.M[3, 2] := -si;
		r.M[3, 3] := co;
		r.IdentityFlag := FALSE;
		T := MMult(T, r);
	end;


(* RotX : modify tranform-Matrix T so that  it will rotate around y-achsis according to phi *)

	procedure RotY (var T: Matrix4; phi: real);

		var
			co, si: real;
			r: Matrix4;

	begin
		co := cos(phi);
		si := sin(phi);
		r := Identity;
		r.M[1, 1] := co;
		r.M[3, 1] := si;
		r.M[1, 3] := -si;
		r.M[3, 3] := co;
		r.IdentityFlag := FALSE;
		T := MMult(T, r);
	end;


(* RotX : modify tranform-Matrix T so that  it will rotate around z-achsis according to phi *)

	procedure RotZ (var T: Matrix4; phi: real);

		var
			co, si: real;
			r: Matrix4;

	begin
		co := cos(phi);
		si := sin(phi);
		r := Identity;
		r.M[1, 1] := co;
		r.M[1, 2] := si;
		r.M[2, 1] := -si;
		r.M[2, 2] := co;
		r.IdentityFlag := FALSE;
		T := MMult(T, r);
	end;

(* RotArbitraryAchsis : ROTATION AROUND ARBITRARY ACHSIS : *)
(* p is a Point on the achsis, x is its Direction (together, p and x form a 3D-Line), phi is the angle *)

	procedure RotArbitraryAchsis (var T: Matrix4; p, x: Vector4; phi: real);

		var
			TAA, TAAI: Matrix4;
			RX, RY: Matrix4;
			TRF: Matrix4;
			RXT, RYT: Matrix4;
			v: Vector4;
			u: Vector4; (* this is the vector defined by p,x *)
			alpha: real; (* rotation angle required to rotate axis to coincide with main axes*)
			beta: real; (* as above *)
			norm: Real; (* norm of vector x-p *)
			a, b, c, d: Real; (* used to calculate sin,cos *)
			cbyd, bbyd: Real; (* as above *)

	begin
{step 1: transform the Achsis so that it will pass through the origin				}
{            this is done by moving the first point, p, to the Origin. calculate the T-Matrix for that }

		TAA := Identity;
		TAAI := Identity;
		RX := Identity;
		RY := Identity;


{ first, calculate the Translation Matrix to move Achsis to Rigin of coordination system }
		TAA.M[4, 1] := -p[1];
		TAA.M[4, 2] := -p[2];
		TAA.M[4, 3] := -p[3];
		TAA.IdentityFlag := FALSE;

{ p[1] ist schon 1, wie TAA[4,4] }
{step 2 : calculate rotation required to rotate rot-achsis to coincidence with one of the axes, here the z-achsis}
{             for this we first rotate around the x-achsis to land in the z-x plane (alpa degrees) : RX }
{             then, in the z-x plane, we will rotate it to coincide with z-achsis (beta degrees) : RY }

		v[1] := x[1] - p[1];
		v[2] := x[2] - p[2];
		v[3] := x[3] - p[3];
		v[4] := 1;

		norm := sqrt(v[1] * v[1] + v[2] * v[2] + v[3] * v[3]);

		u[1] := (v[1]) / norm;
		u[2] := (v[2]) / norm; (* u ist der einheitsvektor in richtung V *)
		u[3] := (v[3]) / norm;
		u[4] := 1;

		a := u[1];
		b := u[2];
		c := u[3]; (* a,b,c sind die richtungscosinuesse (?) von V  projiziert auf normal-koordinaten *)

		d := sqrt(b * b + c * c); (* sei aplpha der winkel, den V mit z-Achse bildet *)

		if d <> 0 then (* wenn d = 0, braucht nicht um x-achse gedreht zu werden *)
			begin
				cbyd := c / d; (* = cos(alpha)  *)
				bbyd := b / d; (* = sin(alpha)  *)
				RX.M[2, 2] := cbyd;
				RX.M[2, 3] := bbyd;
				RX.M[3, 2] := -bbyd;
				RX.M[3, 3] := cbyd; (* durch dies wird V auf z-y-ebene gedreht *)
				RX.IdentityFlag := FALSE;
			end;

		RY.M[1, 1] := d; (* d = cos(beta) *)
		RY.M[1, 3] := a; (* -a = sin(beta) *)
		RY.M[3, 1] := -a;
		RY.M[3, 3] := d;
		RY.IdentityFlag := FALSE;

		TRF := MMult(TAA, RX);
		TRF := MMult(TRF, RY);

{step 3:  now we can rotate for phi degrees around z-achsis normally }

		RotZ(TRF, phi);

{step 4 : Now we have to undo all the other transformations }

		RXT := Transpose(RX); (* for rot-matrix gilt : inv(r) = transp(r) *)

		RYT := Transpose(RY);

		TAAI.M[4, 1] := p[1]; (* dies ist die inverse translate-matrix *)
		TAAI.M[4, 2] := p[2];
		TAAI.M[4, 3] := p[3];
		TAAI.IdentityFlag := FALSE;

		TRF := MMult(TRF, RYT);
		TRF := MMult(TRF, RXT);
		TRF := MMult(TRF, TAAI);

{step 5 : now TRF is the matrix that contains the transformation neccessary. Update T }

		T := MMult(T, TRF);
	end;


{Now the same for Fixed-point arithmetik }

{$ELSEC}
(* Scale : modify transformation-Matrix T so that scaling along x,y,z is according to sx,sy,sz *)

	procedure MScale (var T: Matrix4; sx, sy, sz: real);

		var
			s: Matrix4;

	begin
		s := Identity;
		s.M[1, 1] := X2Fix(sx);
		s.M[2, 2] := X2Fix(sy);
		s.M[3, 3] := X2Fix(sz);
		s.IdentityFlag := FALSE;
		T := MMult(T, s);
	end;


(* Translate : modify tranform-Matrix T so that translation along x,y,z is according to dx,dy,dz *)


	procedure MTranslate (var T: Matrix4; dx, dy, dz: real);

		var
			tl: Matrix4;

	begin
		tl := Identity;
		tl.M[4, 1] := X2Fix(dx);
		tl.M[4, 2] := X2Fix(dy);
		tl.M[4, 3] := X2Fix(dz);
		tl.IdentityFlag := FAlSE;
		T := MMult(T, tl);
	end;


(* RotX : modify tranform-Matrix T so that  it will rotate around x-achsis according to phi *)

	procedure RotX (var T: Matrix4; phi: real);

		var
			co, si: real;
			r: Matrix4;

	begin
		co := cos(phi);
		si := sin(phi);
		r := Identity;
		r.M[2, 2] := X2Fix(co);
		r.M[2, 3] := X2Fix(si);
		r.M[3, 2] := X2Fix(-si);
		r.M[3, 3] := X2Fix(co);
		r.IdentityFlag := FALSE;
		T := MMult(T, r);
	end;


(* RotX : modify tranform-Matrix T so that  it will rotate around y-achsis according to phi *)

	procedure RotY (var T: Matrix4; phi: real);

		var
			co, si: real;
			r: Matrix4;

	begin
		co := cos(phi);
		si := sin(phi);
		r := Identity;
		r.M[1, 1] := X2Fix(co);
		r.M[3, 1] := X2Fix(si);
		r.M[1, 3] := X2Fix(-si);
		r.M[3, 3] := X2Fix(co);
		r.IdentityFlag := FALSE;
		T := MMult(T, r);
	end;


(* RotX : modify tranform-Matrix T so that  it will rotate around z-achsis according to phi *)

	procedure RotZ (var T: Matrix4; phi: real);

		var
			co, si: real;
			r: Matrix4;

	begin
		co := cos(phi);
		si := sin(phi);
		r := Identity;
		r.M[1, 1] := X2Fix(co);
		r.M[1, 2] := X2Fix(si);
		r.M[2, 1] := X2Fix(-si);
		r.M[2, 2] := X2Fix(co);
		r.IdentityFlag := FALSE;
		T := MMult(T, r);
	end;

(* RotArbitraryAchsis : ROTATION AROUND ARBITRARY ACHSIS : *)
(* p is a Point on the achsis, x is its Direction (together, p and x form a 3D-Line), phi is the angle *)

	procedure RotArbitraryAchsis (var T: Matrix4; p, x: Vector4; phi: real);

		var
			TAA, TAAI: Matrix4;
			RX, RY: Matrix4;
			TRF: Matrix4;
			RXT, RYT: Matrix4;
			v: Vector4;
			u: Vector4; (* this is the vector defined by p,x *)
			alpha: Fixed; (* rotation angle required to rotate axis to coincide with main axes*)
			beta: Fixed; (* as above *)
			norm: Fixed; (* norm of vector x-p *)
			a, b, c, d: Fixed; (* used to calculate sin,cos *)
			cbyd, bbyd: Fixed; (* as above *)
			Rnorm: Extended;
			rnx, rny, rnz: real;

	begin
{step 1: transform the Achsis so that it will pass through the origin				}
{            this is done by moving the first point, p, to the Origin. calculate the T-Matrix for that }

		TAA := Identity;
		TAAI := Identity;
		RX := Identity;
		RY := Identity;


{ first, calculate the Translation Matrix to move Achsis to Rigin of coordination system }
		TAA.M[4, 1] := -p[1];
		TAA.M[4, 2] := -p[2];
		TAA.M[4, 3] := -p[3];
		TAA.IdentityFlag := FALSE;

{ p[1] ist schon 1, wie TAA[4,4] }
{step 2 : calculate rotation required to rotate rot-achsis to coincidence with one of the axes, here the z-achsis}
{             for this we first rotate around the x-achsis to land in the z-x plane (alpa degrees) : RX }
{             then, in the z-x plane, we will rotate it to coincide with z-achsis (beta degrees) : RY }

		v[1] := x[1] - p[1];
		v[2] := x[2] - p[2];
		v[3] := x[3] - p[3];
		v[4] := FixRatio(1, 1);

(* ATTENTION: Fixed number resulted in overflow with numbers around 200. So now we *)
(* use the real to calculate the norm. Note that this is not necessary in the norm calc      *)
(* later on since all values have been normalized and their norm should be one                 *)

		rnx := Fix2X(v[1]);
		rny := Fix2X(v[2]);
		rnz := Fix2X(v[3]);
		Rnorm := Sqrt(rnx * rnx + rny * rny + rnz * rnz);
		norm := X2Fix(Rnorm);

(* Above was before calculated with the following statements : *)
(* norm := FixMul(v[1], v[1]) + FixMul(v[2], v[2]) + FixMul(v[3], v[3]); *)
(* norm := X2Fix(Sqrt(Fix2X(norm))); *)
(* norm := X2Fix(sqrt(Fix2X(FixMul(v[1], v[1]) + FixMul(v[2], v[2]) + FixMul(v[3], v[3])))); *)

		u[1] := FixDiv(v[1], norm);
		u[2] := FixDiv(v[2], norm); (* u ist der einheitsvektor in richtung V *)
		u[3] := FixDiv(v[3], norm);
		u[4] := FixRatio(1, 1);

		a := u[1];
		b := u[2];
		c := u[3]; (* a,b,c sind die richtungscosinuesse (?) von V  projiziert auf normal-koordinaten *)

		d := X2Fix(sqrt(Fix2X(FixMul(b, b) + FixMul(c, c)))); (* sei alpha der winkel, den V mit z-Achse bildet *)

		if d <> 0 then (* wenn d = 0, braucht nicht um x-achse gedreht zu werden *)
			begin
				cbyd := FixDiv(c, d); (* = cos(alpha)  *)
				bbyd := FixDiv(b, d); (* = sin(alpha)  *)
				RX.M[2, 2] := cbyd;
				RX.M[2, 3] := bbyd;
				RX.M[3, 2] := -bbyd;
				RX.M[3, 3] := cbyd; (* durch dies wird V auf z-y-ebene gedreht *)
				RX.IdentityFlag := FALSE;
			end;

		RY.M[1, 1] := d; (* d = cos(beta) *)
		RY.M[1, 3] := a; (* -a = sin(beta) *)
		RY.M[3, 1] := -a;
		RY.M[3, 3] := d;
		RY.IdentityFlag := FALSE;

		TRF := MMult(TAA, RX);
		TRF := MMult(TRF, RY);

{step 3:  now we can rotate for phi degrees around z-achsis normally }

		RotZ(TRF, phi);

{step 4 : Now we have to undo all the other transformations }

		RXT := Transpose(RX); (* for rot-matrix gilt : inv(r) = transp(r) *)

		RYT := Transpose(RY);

		TAAI.M[4, 1] := p[1]; (* dies ist die inverse translate-matrix *)
		TAAI.M[4, 2] := p[2];
		TAAI.M[4, 3] := p[3];
		TAAI.IdentityFlag := FALSE;

		TRF := MMult(TRF, RYT);
		TRF := MMult(TRF, RXT);
		TRF := MMult(TRF, TAAI);

{step 5 : now TRF is the matrix that contains the transformation neccessary. Update T }

		T := MMult(T, TRF);
	end;
{$ENDC}

end.