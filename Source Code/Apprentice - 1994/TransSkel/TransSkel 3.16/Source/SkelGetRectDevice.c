/*
 * Given a rectangle, determine the following values:
 * - Which device contains more of the rectangle than any other
 * - The device rectangle (this includes the menu bar area if the device
 * is the main device)
 * - Whether or not the device is the main device
 *
 * These values are stuffed into the arguments, which are passed as
 * variable addresses.  If you're not interested in a particular value,
 * pass nil for the corresponding argument.
 *
 * The return value if true if the rectangle overlaps some device,
 * false if it lies outside all devices.  If the rectangle overlaps no
 * device, non-nil arguments are filled in with the main device, the main
 * device rect, and true, respectively.  This is useful, e.g., for callers
 * that may want to reposition a window if its content rectangle isn't
 * visible on some monitor.
 *
 * The returned device value will be nil on systems that don't have GDevices
 * (i.e.,, that don't support Color QuickDraw), even if the function result
 * is true.
 *
 * References: TN TB 30.
 */

# include	"TransSkel.h"


pascal Boolean
SkelGetRectDevice (Rect *rp, GDHandle *rGD, Rect *devRect, Boolean *isMain)
{
GDHandle	gd, curGD;
Rect		gdRect, curRect, iSectRect;
long		maxArea, area;
Boolean		main = false;
Boolean		result;
		
	gd = (GDHandle) nil;				/* no device for rectangle known yet */

	if (!SkelQuery (skelQHasColorQD))
	{
		/*
		 * No Color QuickDraw implies only one screen, which is therefore
		 * the main device.  Test rectangle against full screen, setting
		 * result true if they intersect.
		 */
		main = true;
		gdRect = screenBits.bounds;
		result = SectRect (rp, &gdRect, &iSectRect);
	}
	else
	{
		/* determine device having maximal overlap with r */

		maxArea = 0;
		for (curGD = GetDeviceList (); curGD != (GDHandle) nil; curGD = GetNextDevice (curGD))
		{
			/* only consider active screen devices */
			if (!TestDeviceAttribute (curGD, screenDevice)
					|| !TestDeviceAttribute (curGD, screenActive))
				continue;
			curRect = (**curGD).gdRect;
			if (!SectRect (rp, &curRect, &iSectRect))
				continue;
			area = (long) (iSectRect.right - iSectRect.left)
					* (long) (iSectRect.bottom - iSectRect.top);
			if (maxArea < area)
			{
				maxArea = area;
				gd = curGD;
				gdRect = curRect;
				result = true;	/* rectangle overlaps some device */
			}
		}
		if (gd == (GDHandle) nil)	/* rectangle overlaps no device, use main */
		{
			gd = GetMainDevice ();
			gdRect = (**gd).gdRect;
			result = false;
		}
		main = (gd == GetMainDevice ());
	}

	/* fill in non-nil arguments */

	if (rGD != (GDHandle *) nil)
		*rGD = gd;
	if (devRect != (Rect *) nil)
		*devRect = gdRect;
	if (isMain != (Boolean *) nil)
		*isMain = main;

	return (result);
}
