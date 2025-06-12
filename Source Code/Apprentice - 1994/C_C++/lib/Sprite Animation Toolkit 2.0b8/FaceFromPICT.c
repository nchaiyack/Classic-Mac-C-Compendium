// C translation by Adam Wight

#include "SAT.h"
//SATGlobals, DisposeOffscreen, Offscreen;/*SAT*/

FacePtr GetFaceFromPICT (int colorPICTid,int bwPICTid,int maskPICTid);

/*No error checking yet!*/

FacePtr GetFaceFromPICT (int colorPICTid,int bwPICTid,int maskPICTid) {
        Rect bounds;
        PicHandle thePICT, maskPICT;
        FacePtr theFace;
        GrafPtr savePort;
        GDHandle saveGD;

        GetPort(savePort);
        if (colorFlag) saveGD = GetGDevice();

        /*Get PICTs*/
        if (gSAT.initDepth > 1) thePICT = GetPicture(colorPICTid);
        else thePICT = GetPicture(bwPICTid);

        maskPICT = GetPicture(maskPICTid);
        bounds = (*thePICT)->picFrame;
        OffsetRect(&bounds, -bounds.left, -bounds.top); /* Unnecessary? I think NewFace does this for us. /Ingemar */

        /*Create face*/
        theFace = NewFace(bounds);

        /*Draw in the face*/
        SetPortFace(theFace);
        DrawPicture(thePICT, &bounds);
        FrameRect(&bounds);
        SetPortMask(theFace);
        DrawPicture(maskPICT, &bounds);
        FrameRect(&bounds);
        /*Tell SAT that we are done*/
        ChangedFace(theFace);

        /*Get rid of the PICTs*/
        ReleaseResource((Handle)thePICT);
        ReleaseResource((Handle)maskPICT);

        /*Return the face.*/

        if (colorFlag) SetGDevice(saveGD);
        SetPort(savePort);

        return theFace;
}
