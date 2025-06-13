/* FastModulation.c */
/*****************************************************************************/
/*                                                                           */
/*    Out Of Phase:  Digital Music Synthesis on General Purpose Computers    */
/*    Copyright (C) 1994  Thomas R. Lawrence                                 */
/*                                                                           */
/*    This program is free software; you can redistribute it and/or modify   */
/*    it under the terms of the GNU General Public License as published by   */
/*    the Free Software Foundation; either version 2 of the License, or      */
/*    (at your option) any later version.                                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          */
/*    GNU General Public License for more details.                           */
/*                                                                           */
/*    You should have received a copy of the GNU General Public License      */
/*    along with this program; if not, write to the Free Software            */
/*    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.              */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Audit.h"
#include "Debug.h"
#include "Definitions.h"

#include "FastModulation.h"
#include "Memory.h"


/* apply a modulation chain to a value.  all vectors are of length NumModulators */
/* except for OldValueVector whose length is the number of oscillators.  each */
/* oscillator is looked up as OldValueVector[IndirectionTable[i]] */
float									ApplyModulation(float Value, ModulationTypes* ModulateHow,
												float* ModulationScaling, float* ModulationOrigin,
												float* OldValueVector, long* IndirectionTable,
												long NumModulators)
	{
		long								Scan;

		for (Scan = 0; Scan < NumModulators; Scan += 1)
			{
				float								Modulus;

				PRNGCHK(ModulateHow,&(ModulateHow[Scan]),sizeof(ModulateHow[Scan]));
				PRNGCHK(ModulationScaling,&(ModulationScaling[Scan]),
					sizeof(ModulationScaling[Scan]));
				PRNGCHK(ModulationOrigin,&(ModulationOrigin[Scan]),
					sizeof(ModulationOrigin[Scan]));
				PRNGCHK(IndirectionTable,&(IndirectionTable[Scan]),
					sizeof(IndirectionTable[Scan]));
				PRNGCHK(OldValueVector,&(OldValueVector[IndirectionTable[Scan]]),
					sizeof(OldValueVector[IndirectionTable[Scan]]));
				Modulus = OldValueVector[IndirectionTable[Scan]]
					* ModulationScaling[Scan] + ModulationOrigin[Scan];
				switch (ModulateHow[Scan])
					{
						default:
							EXECUTE(PRERR(ForceAbort,"ApplyModulation:  bad modulation type"));
							break;
						case eModulationAdditive:
							/* y = x + (m * scale + const) */
							Value = Value + Modulus;
							break;
						case eModulationMultiplicative:
							/* y = x * (m * scaling + const) */
							/* this can be transformed into */
							/* y = x * (1 - m * scaling + const) */
							/* by doing this:  const' = 1 + const, scaling' = - scaling */
							/* y = x * (m * (-scaling) + (1 + const)) */
							/* y = x * (m * scaling' + const') */
							Value = Value * Modulus;
							break;
					}
			}
		return Value;
	}
