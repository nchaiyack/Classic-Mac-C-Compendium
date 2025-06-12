#ifndef _REGIONS_H_
#define _REGIONS_H_

// InitRegionMaker.
// Initialize the region-making package; the conversion from picts
// to regions requires an offscreen grafport buffer, thus requiring
// this call.
OSErr InitRegionMaker(Rect *screenSize, ResType rgnType = 'Rgn ');


// SetRegionType.
// Sets the resource type of the converted regions, i.e.
// from 'PICT' to 'Rgn ', 'RGN ', 'RgnS', etc.
void SetRegionType(ResType rgnType);


// GetRegionType.
// Returns the current resource type of regions.
ResType GetRegionType();


// PictureFromScrap.
// Get the picture from the scrap (clipboard), if any. If no
// picture, returns nil.
PicHandle PictureFromScrap();


// DataFromScrap.
// Returns the requested resource type from the scrap/clipboard. If no
// resource of the specified type is present, returns nil.
Handle DataFromScrap(ResType dataType);


// RegionToScrap.
// Given a handle to a valid region, this routine "pastes" the region
// onto the scrap (clipboard). Returns an error code if anything goes
// wrong. Calls _ZeroScrap() before putting the region onto the clipboard,
// so anything there previously is erased.
OSErr RegionToScrap(RgnHandle theRgn);


// ConvertPictureToRegion
// Here's the main man, the core guts, the head honcho, the
// mustafa guy, the rastafarian, you get the idea. Calls _BitMapToRegion()
// to do the actual dirty work.
RgnHandle ConvertPictureToRegion(PicHandle thePicture, Boolean doOffset = true);


// PreviewRgn.
// Given a valid region, puts up its own window and shows what the
// region looks like, waits for an event (mouse, key) and then
// disposes of the window.
void PreviewRgn(RgnHandle theRgn);


#endif // _REGIONS_H_