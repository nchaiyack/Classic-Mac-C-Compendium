/* FastModulation.h */

#ifndef Included_FastModulation_h
#define Included_FastModulation_h

/* FastModulation module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* ModulationSpecifier */

#include "ModulationSpecifier.h"

/* unfortunately, due to the very restrictive range of fast fixedpoint numbers */
/* (i.e. -1..1), we need to use floating point values for modulation. */

/* apply a modulation chain to a value.  all vectors are of length NumModulators */
/* except for OldValueVector whose length is the number of oscillators.  each */
/* oscillator is looked up as OldValueVector[IndirectionTable[i]] */
float									ApplyModulation(float Value, ModulationTypes* ModulateHow,
												float* ModulationScaling, float* ModulationOrigin,
												float* OldValueVector, long* IndirectionTable,
												long NumModulators);

#endif
