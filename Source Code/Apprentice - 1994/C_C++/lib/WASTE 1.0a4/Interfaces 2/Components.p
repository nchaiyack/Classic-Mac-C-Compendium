unit Components;

{ Pascal Interface to the Macintosh Libraries }

{ Copyright © 1990-1993 Apple Computer, Inc. }

{ This file has been processed by the THINK Pascal Source Converter, v. 1.1.2 }
{ and further adapted for use in conjunction with THINK Pascal }

interface

	const
		gestaltComponentMgr = 'cpnt';

		kAnyComponentType = 0;
		kAnyComponentSubType = 0;
		kAnyComponentManufacturer = 0;
		kAnyComponentFlagsMask = 0;

		cmpWantsRegisterMessage = $80000000;

{ Component Resource Extension flags }
		componentDoAutoVersion = 1;
		componentWantsUnregister = 2;
		componentAutoVersionIncludeFlags = 4;

	type
		ComponentDescription = record
				componentType: OSType;								{ A unique 4-byte code indentifying the command set }
				componentSubType: OSType;							{ Particular flavor of this instance }
				componentManufacturer: OSType;						{ Vendor indentification }
				componentFlags: LONGINT;							{ 8 each for Component,Type,SubType,Manuf/revision }
				componentFlagsMask: LONGINT;						{ Mask for specifying which flags to consider in search, zero during registration }
			end;

		ResourceSpec = record
				resType: OSType;									{ 4-byte code  }
				resId: INTEGER;									{    }
			end;

		ComponentResourcePtr = ^ComponentResource;
		ComponentResourceHandle = ^ComponentResourcePtr;
		ComponentResource = record
				cd: ComponentDescription;							{ Registration parameters }
				component: ResourceSpec;							{ resource where Component code is found }
				componentName: ResourceSpec;						{ name string resource }
				componentInfo: ResourceSpec;						{ info string resource }
				componentIcon: ResourceSpec;						{ icon resource }
			end;

		ComponentResourceExtension = record
				componentVersion: LONGINT;			{ version of Component }
				componentRegisterFlags: LONGINT;			{ flags for registration }
				componentIconSuite: INTEGER;			{ resource id of Icon Suite }
			end;

		Component = ^ComponentRecord;
		ComponentRecord = record
				data: array[0..0] of LONGINT;
			end;

		ComponentInstance = ^ComponentInstanceRecord;
		ComponentInstanceRecord = record
				data: array[0..0] of LONGINT;
			end;

{ Structure received by Component:  }
		ComponentParameters = packed record
				flags: CHAR;										{ call modifiers: sync/async, deferred, immed, etc }
				paramSize: CHAR;									{ size in bytes of actual parameters passed to this call }
				what: INTEGER;										{ routine selector, negative for Component management calls }
				params: array[0..0] of LONGINT;					{ actual parameters for the indicated routine }
			end;

		ComponentResult = LONGINT;

		ComponentRoutine = ProcPtr;

		ComponentFunction = ProcPtr;


{******************************************************}
{* Required Component routines}
{******************************************************}

	const
		kComponentOpenSelect = -1;							{ ComponentInstance for this open }
		kComponentCloseSelect = -2;							{ ComponentInstance for this close }
		kComponentCanDoSelect = -3;							{ selector # being queried }
		kComponentVersionSelect = -4;						{ no params }
		kComponentRegisterSelect = -5;						{ no params }
		kComponentTargetSelect = -6;						{ ComponentInstance for top of call chain }
		kComponentUnregisterSelect = -7;					{ no params }

{ Set Default Component flags }
		defaultComponentIdentical = 0;
		defaultComponentAnyFlags = 1;
		defaultComponentAnyManufacturer = 2;
		defaultComponentAnySubType = 4;
		defaultComponentAnyFlagsAnyManufacturer = defaultComponentAnyFlags + defaultComponentAnyManufacturer;
		defaultComponentAnyFlagsAnyManufacturerAnySubType = defaultComponentAnyFlags + defaultComponentAnyManufacturer + defaultComponentAnySubType;

{ errors from component manager & components }
		invalidComponentID = -3000;
		validInstancesExist = -3001;
		componentNotCaptured = -3002;
		componentDontRegister = -3003;

		badComponentInstance = $80008001;
		badComponentSelector = $80008002;


{ *******************************************************}
{*                                                     *}
{*              APPLICATION LEVEL CALLS                *}
{*                                                     *}
{*******************************************************}
{* Component Database Add, Delete, and Query Routines }
{*******************************************************}
{}
	function RegisterComponent (cd: ComponentDescription;
									componentEntryPoint: ComponentRoutine;
									global: INTEGER;
									componentName: Handle;
									componentInfo: Handle;
									componentIcon: Handle): Component;
	inline
		$7001, $A82A;
	function RegisterComponentResource (tr: ComponentResourceHandle;
									global: INTEGER): Component;
	inline
		$7012, $A82A;
	function UnregisterComponent (aComponent: Component): OSErr;
	inline
		$7002, $A82A;

	function FindNextComponent (aComponent: Component;
									looking: ComponentDescription): Component;
	inline
		$7004, $A82A;
	function CountComponents (looking: ComponentDescription): LONGINT;
	inline
		$7003, $A82A;

	function GetComponentInfo (aComponent: Component;
									var cd: ComponentDescription;
									componentName: Handle;
									componentInfo: Handle;
									componentIcon: Handle): OSErr;
	inline
		$7005, $A82A;
	function GetComponentListModSeed: LONGINT;
	inline
		$7006, $A82A;


{ *******************************************************}
{* Component Instance Allocation and dispatch routines }
{*******************************************************}
{}
	function OpenComponent (aComponent: Component): ComponentInstance;
	inline
		$7007, $A82A;
	function CloseComponent (aComponentInstance: ComponentInstance): OSErr;
	inline
		$7008, $A82A;

	function GetComponentInstanceError (aComponentInstance: ComponentInstance): OSErr;
	inline
		$700A, $A82A;


{  direct calls to the Components  }
	function ComponentFunctionImplemented (ci: ComponentInstance;
									ftnNumber: INTEGER): LONGINT;
	inline
		$2F3C, $2, $FFFD, $7000, $A82A;
	function GetComponentVersion (ci: ComponentInstance): LONGINT;
	inline
		$2F3C, $0, $FFFC, $7000, $A82A;
	function ComponentSetTarget (ci: ComponentInstance;
									target: ComponentInstance): LONGINT;
	inline
		$2F3C, $4, $FFFA, $7000, $A82A;


{****************************************************}
{*                                                    *}
{*               CALLS MADE BY Components             *}
{*                                                    *}
{******************************************************}


{ *******************************************************}
{* Component Management routines}
{*******************************************************}
{}
	procedure SetComponentInstanceError (aComponentInstance: ComponentInstance;
									theError: OSErr);
	inline
		$700B, $A82A;

	function GetComponentRefcon (aComponent: Component): LONGINT;
	inline
		$7010, $A82A;
	procedure SetComponentRefcon (aComponent: Component;
									theRefcon: LONGINT);
	inline
		$7011, $A82A;

	function OpenComponentResFile (aComponent: Component): INTEGER;
	inline
		$7015, $A82A;
	function CloseComponentResFile (refnum: INTEGER): OSErr;
	inline
		$7018, $A82A;


{ *******************************************************}
{* Component Instance Management routines}
{*******************************************************}
{}
	function GetComponentInstanceStorage (aComponentInstance: ComponentInstance): Handle;
	inline
		$700C, $A82A;
	procedure SetComponentInstanceStorage (aComponentInstance: ComponentInstance;
									theStorage: Handle);
	inline
		$700D, $A82A;

	function GetComponentInstanceA5 (aComponentInstance: ComponentInstance): LONGINT;
	inline
		$700E, $A82A;
	procedure SetComponentInstanceA5 (aComponentInstance: ComponentInstance;
									theA5: LONGINT);
	inline
		$700F, $A82A;

	function CountComponentInstances (aComponent: Component): LONGINT;
	inline
		$7013, $A82A;

{  useful helper routines for convenient method dispatching  }
	function CallComponentFunction (params: ComponentParameters;
									func: ComponentFunction): LONGINT;
	inline
		$70FF, $A82A;
	function CallComponentFunctionWithStorage (storage: Handle;
									params: ComponentParameters;
									func: ComponentFunction): LONGINT;
	inline
		$70FF, $A82A;
	function DelegateComponentCall (originalParams: ComponentParameters;
									ci: ComponentInstance): LONGINT;
	inline
		$7024, $A82A;

	function SetDefaultComponent (aComponent: Component;
									flags: INTEGER): OSErr;
	inline
		$701E, $A82A;
	function OpenDefaultComponent (componentType: OSType;
									componentSubType: OSType): ComponentInstance;
	inline
		$7021, $A82A;
	function CaptureComponent (capturedComponent: Component;
									capturingComponent: Component): Component;
	inline
		$701C, $A82A;
	function UncaptureComponent (aComponent: Component): OSErr;
	inline
		$701D, $A82A;
	function RegisterComponentResourceFile (resRefNum: INTEGER;
									global: INTEGER): LONGINT;
	inline
		$7014, $A82A;
	function GetComponentIconSuite (aComponent: Component;
									var iconSuite: Handle): OSErr;
	inline
		$702A, $A82A;

implementation
end.