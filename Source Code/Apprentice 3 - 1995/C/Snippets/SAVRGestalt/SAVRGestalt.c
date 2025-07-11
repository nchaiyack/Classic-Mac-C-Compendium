/* ================================================= */
/* ================== SAVRGestalt.c ================ */
/* ================================================= */

#include <GestaltEqu.h>    // for Gestalt
#include <A4Stuff.h>       // for globals in your code resource
#include <OSUtils.h>

// These were defined in AfterDarkGestalt.h
// I don't remember why I didn't just #include it
#define kMySaverOn    0x00000001
#define kMySaverOff   0x00000000

#define NIL          0L

// type definition for the SAVR Gestalt function
typedef pascal OSErr (*GestaltFunctionPtr) (OSType gestaltSelector,
                                            long *gestaltResponse);

Boolean     gSaverOn;  // TRUE if you want this Gestalt to work, FALSE otherwise
long     gOldGestaltAddr, gMyAddr;  // addresses of important stuff

/*
   main() gets called when you call the code resource.  In my case, main
   has been set up to return the addresses of the other two functions
   in the code resource, so that the program can call them.
*/
void main(long *gestaltAddr, long *setVarsAddr);

/*
   SetVars() allows the program to set globals in the code resource.
   You can pass NIL in the two longs if you don't want to change
   those values.
*/
void SetVars(Boolean saverOn, long oldGestaltAddr, long myAddr);

/*
   myGestaltFunc() is the actual Gestalt handler.  This function actually
   handles two different Gestalts:  the SAVR Gestalt, and my custom Gestalt,
   named by the creator ID of my program.  The function knows which is
   being handled by looking at the gestaltSelector parameter.
*/
pascal OSErr myGestaltFunc(OSType gestaltSelector, long *gestaltResponse);

void main(long *gestaltAddr, long *setVarsAddr)
{
  long   oldA4;

  oldA4 = SetCurrentA4();
  *gestaltAddr = ((long) myGestaltFunc);
  *setVarsAddr = ((long) SetVars);
  SetA4(oldA4);
}

void SetVars(Boolean saverOn, long oldGestaltAddr, long myAddr)
{
  long   oldA4;

  oldA4 = SetCurrentA4();
  gSaverOn = saverOn;
  if (oldGestaltAddr != NIL)
    gOldGestaltAddr = oldGestaltAddr;
  if (myAddr != NIL)
    gMyAddr = myAddr;
  SetA4(oldA4);
}

pascal OSErr myGestaltFunc(OSType gestaltSelector, long *gestaltResponse)
{
  OSErr     err;
  long      oldA4;

  oldA4 = SetCurrentA4();

  if (gestaltSelector == 'SAVR')
  {
    if (gSaverOn)  // meaning that I need to handle this Gestalt
    {
      *gestaltResponse = kMySaverOn;
      err = noErr;
    }
    else  // meaning that I need to call the Gestalt I replaced, if any
    {
      // if it's NIL, my SAVR Gestalt wan't installed over another SAVR Gestalt
      if (gOldGestaltAddr != NIL)
      {
        // call the SAVR Gestalt that we replaced
        err = ((GestaltFunctionPtr) gOldGestaltAddr) (gestaltSelector,
                                                      gestaltResponse);
      }
      else  // I didn't replace one, so the saver is off
      {
        *gestaltResponse = kMySaverOff;
        err = noErr;
      }
    }
  }

  // change '????' to whatever your program's creator is.
  // Then, by calling Gestalt with that code, the return value will
  // contain the address of the code resource.
  if (gestaltSelector == '????')
  {
    *gestaltResponse = gMyAddr;  // return address of code resource
    err = noErr;
  }

  SetA4(oldA4);
  return err;
}

/* ============ end SAVRGestalt.c ============ */


Next, in your program, you include the following type definitions:

typedef pascal OSErr (*GestaltFunctionPtr) (OSType gestaltSelector,
                                            long *gestaltResponse);
typedef void (*GestaltInfoPtr) (long *gestaltAddr, long *setVarsAddr);
typedef void (*GestaltSetVars) (Boolean saverOn, long oldGestaltAddr,
                                long myAddr);

The GestaltFunctionPtr is a pointer to a Gestalt selector function, the
GestaltInfoPtr is a pointer to the main() function from the code resource,
and GestaltSetVars is a pointer to the SetVars() function from the code
resource.

Lastly, you'll want to declare the following functions:

void InstallGestalt(void)
{
  OSErr     gestaltErr;    // error returned by Gestalt
  Handle gstFuncHandle;    // handle to Gestalt function
  long      gestaltSelectorFunc;
  long      result, setVarsProc;
  ProcPtr   oldGestaltFunc = NIL;   // pointer to old Gestalt selector function

  // replace '????' with your creator ID
  // The purpose of this call is to check and see if the code resource has
  // already been installed.  If it has, gestaltErr returns noErr, and the
  // result parameter contains a pointer to main() in the code resource.
  // We go to this trouble because the program might have quit and restarted,
  // in which case we don't know what's going on.
  gestaltErr = Gestalt('????', &result);

  if (gestaltErr == noErr)
  {
    // call main(), returning addresses of myGestaltFunc() and SetVars()
    ((GestaltInfoPtr) result) (&gestaltSelectorFunc, &setVarsProc);
    // turn on our selector, without changing the addresses
    ((GestaltSetVars) setVarsProc) (TRUE, NIL, NIL);
  }
  else  // it hasn't been installed, so let's install it
  {
    gstFuncHandle = GetResource('GDEF', 128);
    if (ResError())
      DeathAlert(errResource);
    // DeathAlert() is my own error function.  The parameters are my own consts,
    // and are defined elsewhere.  DeathAlert() calls ExitToShell at the end.
    if ((gstFuncHandle == NIL) || (*gstFuncHandle == NIL))
      DeathAlert(errNoFindGDEF);

    DetachResource(gstFuncHandle);
    if (ResError())
      DeathAlert(errResource);

  // We now call the function pointed to by the handle returned by GetResource.
  // This seems strange -- keep in mind that this is really just the main()
  // function.  We're calling it to get all the info we need.
    ((GestaltInfoPtr) *gstFuncHandle) (&gestaltSelectorFunc, &setVarsProc);

    gestaltErr = NewGestalt('SAVR',
                           NewSelectorFunctionProc(gestaltSelectorFunc));
    if (gestaltErr != noErr)
    {
    // Could not add SAVR as a new selector.  Try to replace existing
selector
      gestaltErr = ReplaceGestalt('SAVR',
                           NewSelectorFunctionProc(gestaltSelectorFunc),
                           (SelectorFunctionProcPtr *) &oldGestaltFunc);
      if (gestaltErr != noErr)
        DeathAlert(errNoInstallSAVR);
    }
    ((GestaltSetVars) setVarsProc) (TRUE, (long) oldGestaltFunc,
                                    (long) *gstFuncHandle);
    /* TRUE to specify saver is on */
    /* gOldGestaltFunc will be NIL if it wasn't replaced. */
    /* *gstFuncHandle holds pointer to resource, which is locked, so we don't */
    /*     have to worry about the dereferenced handle becoming invalid. */

    gestaltErr = NewGestalt('????',
                           NewSelectorFunctionProc(gestaltSelectorFunc));
    if (gestaltErr != noErr)
      DeathAlert(errNoInstallCafe);
  }
}

void RemoveGestalt(void)
{
  OSErr     err;
  long      response, selectorProc, setVarsProc;
  ProcPtr   temp;

  // once again, we must assume that the program could have quit, so
  // we don't know what's going on.  The rest should look very familiar
  // if you've looked at the InstallGestalt() function.
  err = Gestalt('????', &response);
  if (err != noErr)
    DeathAlert(errNoFindCafe);

  ((GestaltInfoPtr) response) (&selectorProc, &setVarsProc);
  ((GestaltSetVars) setVarsProc) (FALSE, NIL, NIL);
}



