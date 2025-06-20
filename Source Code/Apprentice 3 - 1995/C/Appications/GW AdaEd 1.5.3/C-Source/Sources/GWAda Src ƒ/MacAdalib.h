/*
 * MacAdalib.h - structures to read in the library
 *
 * AdaEd - Mac version
 *
 */
/*
 * Copyright (C) 1985-1992  New York University
 * Copyright (C) 1994 George Washington University
 * 
 * This file is part of the GWAdaEd system, an extension of the Ada/Ed-C
 * system.  See the Ada/Ed README file for warranty (none) and distribution
 * info and also the GNU General Public License for more details.
 */


#pragma once

typedef enum {
	kSubUnit,
	kPackageSpec,
	kPackageBody,
	kSubprogramSpec,
	kSubprogram,
	kBindingUnit,
	kUnit,
	kPackageTaskStub,
	kSubprogramStub,
	kStub
} UnitKind;

typedef enum {
	kNotMain,
	kMainInterface,
	kMain
} MainKind;

typedef enum {
	kActive,
	kObsolete
} CompStatus;

typedef struct {
	char	*uname;			// "lib-unit-name"
	UnitKind	ukind;
	int		unumber;		// "lib-unit-number"
	char	*aisname;		// "lib-ais-name"
	char	comp_date[15];	// "unit-date"

	int		symbols;		// "lib-symbols"
	int		nodes;			// "lib-nodes"
	int		is_main;		// "lib-is-main"

	MainKind	mkind;
		// mkind: kNotMain use ""
		//        kMainInterface use "(Interface)"
		//        kMain use "(Main)"

	CompStatus	cstatus;	// "lib-status"

} LibUnit;

typedef struct {
	char	*uname;			// "lib-unit-name"
			// formatted_stub(uname)
	UnitKind	stype;

	char	*aisname;		// "lib-ais-name"
	int		parent;			// "lib-parent"
	int		cur_level;		// "lib-cur-level"
	int		m;				// "stub-file-size"

	//int		*ignore;	// [m]

} Stubs;

typedef struct {
	int		dom;
	int		m;
	int		*range;
} PrecMapSize;

typedef struct {
	int		exists;			// "slot-exists"

	// if (exists) then
	int		slot_seq;		// "slot-seq"
	int		slot_unit;		// "slot-unit"
	int		slot_number;	// "slot-number"
	char	*slot_name;		// "slot_name"
#ifdef MONITOR
	char	*slot_file;		// "slot_file"
#endif

	// endif
} SlotEntry;

typedef struct {
	int		numEntries;		// "slot-entries"
	SlotEntry	*entries;		// [numEntries]
} LSlot;

typedef struct {
	int		unit_count;			// "lib-unit-count"
	int		n1;					// "lib-n"
	int		empty_unit_slots;	// "lib-empty-slots"

	LibUnit	*units;		// [unit_count]

	int		stubs_count;		// "lib-n"
	Stubs	*stubs;	// [stubs_count]

	int		n_code;				// "n-code"
	int		n_data;				// "n-data"
	int		n_exception;		// "n-exception"

	LSlot	code;
	LSlot	data;
	LSlot	exceptions;
} Library;




/* Prototype */
int		adalib(char *fullPath, Library *lib);
void	freeadalib(Library lib);

void	formatted_name(LibUnit *unit, char *name);
void	formatted_stub(Stubs *unit, char *name);
