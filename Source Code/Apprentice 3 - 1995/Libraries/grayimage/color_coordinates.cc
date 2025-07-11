// This may look like C code, but it is really -*- C++ -*-
/*
 *************************************************************************
 *
 *	     Demostrating representation of the color picture
 *		     in various color coordinates
 *
 * There are a lot of color coordinates schemes besides the familiar
 * RGB. The present programs reads an image (written in the PPM,
 * portable pixel map, which is RGB color coded) and displays the image
 * in color as well as its three color projections (coordinates)
 * for a variety of color coordinate systems.
 * See the comments below for the color coordinates.
 *
 *************************************************************************
 */ 

#include "image.h"
#include <fstream.h>


/*
 *------------------------------------------------------------------------
 *		  Representation of the color image
 */

class ColorImage {

protected:
  char * color_scheme_label;		// Label for the color scheme
  char * coord_labels[3];		// Labels for the color coordinates
  IMAGE * color_proj[3];		// 3 color projections as gray-scale
					// images
  ColorImage(void) {}			// All actual creation is to be done
					// in siblings

public:
  ~ColorImage(void);

  char * q_color_scheme_label(void) const 	{ return color_scheme_label; }
  char * q_coord_label(const int i) const
  { assert( i>=0 && i<3 ); return coord_labels[i]; }

  IMAGE& proj_image(const int i)
  { assert( i>=0 && i<3 ); return *color_proj[i]; }

  void display_projections(void) const;
};


				// Common destructor
ColorImage::~ColorImage(void)
{
  assert( color_scheme_label != 0 && color_proj[0] != 0 );
  delete color_scheme_label;
  register int i;
  for(i=0; i<3; i++)
    delete coord_labels[i], delete color_proj[i];
}

				// Display color projections of the image
void ColorImage::display_projections(void) const
{
  char buffer[200];
  register int i;
  for(i=0; i<3; i++)
  {
    sprintf(buffer,"%s/%s",color_scheme_label,coord_labels[i]);
    (*color_proj[i]).display(buffer);
  }
}

/*
 *------------------------------------------------------------------------
 *			RGB image and reading it
 */

class RGB_image : public ColorImage {

public:
				// Read the color image from the ppm file
  RGB_image(const char * ppm_file_name);
  ~RGB_image(void) {}		// Nothing special in destruction
};

RGB_image::RGB_image(const char * ppm_file_name)
{
  message("\nReading the PPM image from '%s'...\n",ppm_file_name);
  ifstream fp(ppm_file_name);
  char c;
  if( !fp.get(c) || c != 'P' || !fp.get(c) || c != '6' )
    _error("Unable to read the correct header 'P6' of the compact PPM");

  int width, height, depth;
  fp >> width >> height >> depth;
  depth = log2(depth+1);		// Convert from maxvalue to depth
  if( !fp.get(c) || c != '\n' )
    _error("Bad symbol %xh occured after the depth specification",c);

  color_scheme_label = strdup("RGB NTSC");
  coord_labels[0] = strdup("Red");
  coord_labels[1] = strdup("Green");
  coord_labels[2] = strdup("Blue");

  register int pi;
  for(pi=0; pi<3; pi++)
    color_proj[pi] = new IMAGE(height,width,depth);

  message("\nReading the RGB pixel matrix for %dx%dx%d color image ...",
	  width,height,depth);
  register int i,j;
  for(i=0; i<height; i++)
    for(j=0; j<width; j++)
      if( !fp.get(c) || ((*color_proj[0])(i,j) = c,0) ||
	  !fp.get(c) || ((*color_proj[1])(i,j) = c,0) ||
	  !fp.get(c) || ((*color_proj[2])(i,j) = c,0) )
	_error("Error reading the PPM file",(perror(""),0));
  message("Done\n\n");
}		

/*
 *------------------------------------------------------------------------
 *	Cobversion from RGB to other tristimulus color system
 *
 * See Pratt, Digital Image processing, Appendix 2
 * Note, that elements of the conversion matrix are float point numbers
 * in [0,2]. We represent them as rational numbers with the common
 * denominator specified separately.
 *
 */

struct TristimulusSystem {

  char * label;				// Label of the system
  char * tristimulus_labels[3];		// Labels for the tristimulus
  int  conv_matrix [3] [3];		// Conversion matrix from RGB
  int  common_denominator;
};


class TristimulusImage : public ColorImage {

public:
				// Convert from RGB to other system
  TristimulusImage(const RGB_image& rgbim, const TristimulusSystem& conv);
  ~TristimulusImage() {}
};

				// Convert from RGB to other system
TristimulusImage::TristimulusImage
(const RGB_image& rgbim, const TristimulusSystem& conv)
{
  color_scheme_label = strdup(conv.label);

  register int i;
  for(i=0; i<3; i++)
    coord_labels[i] = strdup(conv.tristimulus_labels[i]),
    color_proj[i] = new IMAGE(*rgbim.color_proj[0]);

					// Actual conversion
  register int j;
  for(i=0; i<(*color_proj[0]).q_nrows(); i++)
    for(j=0; j<(*color_proj[0]).q_ncols(); j++)
    {
      const int rval = (*rgbim.color_proj[0])(i,j);
      const int gval = (*rgbim.color_proj[1])(i,j);
      const int bval = (*rgbim.color_proj[2])(i,j);
      (*color_proj[0])(i,j) = ( rval*conv.conv_matrix[0][0] +
			     gval*conv.conv_matrix[0][1] +
			     bval*conv.conv_matrix[0][2] ) 
			   / conv.common_denominator;
      (*color_proj[1])(i,j) = ( rval*conv.conv_matrix[1][0] +
		        gval*conv.conv_matrix[1][1] +
		        bval*conv.conv_matrix[1][2] ) 
	              / conv.common_denominator;
      (*color_proj[2])(i,j) = ( rval*conv.conv_matrix[2][0] +
		        gval*conv.conv_matrix[2][1] +
		        bval*conv.conv_matrix[2][2] ) 
	              / conv.common_denominator;
    }
}


typedef void (*ProcessingFunction)(const RGB_image& image,
				   const void * argument);

static void convert_to_tristimulus_system
	(const RGB_image& rgb_image, const TristimulusSystem * conv)
{
  TristimulusImage new_image(rgb_image,*conv);
  new_image.display_projections();
}

/*
 *------------------------------------------------------------------------
 *		Conversion from tristimulus to the
 *		luminance-chrominance color system
 *
 * If T1, T2, and T3 are three tristimulus values in some coordinate
 * system, then chromacity values can be defined as follows
 *
 *	t1 = T1/(T1+T2+T3)
 *	t2 = T2/(T1+T2+T3)
 * 
 * Chrominance values, which are normally within the range [0,1] are
 * multiplied by 255 for display
 *
 */

static void convert_to_chrominance
	(const RGB_image& rgb_image, const TristimulusSystem * conv)
{
  TristimulusImage T_image(rgb_image,*conv);
  IMAGE t1(T_image.proj_image(0));		// Chrominance components
  IMAGE t2(t1);
  t1 = T_image.proj_image(0);
  t2 = T_image.proj_image(1);
  IMAGE& T3 = T_image.proj_image(2);

  register int i,j;
  for(i=0; i<t1.q_nrows(); i++)
    for(j=0; j<t1.q_ncols(); j++)
    {
      const int T3v = T3(i,j);
      register GRAY& t1r = t1(i,j);
      register GRAY& t2r = t2(i,j);
      register int sum = t1r + t2r + T3v;		// T1+T2+T3
      if( sum == 0 )
	t1r = 0, t2r = 0;				// Black pixel
      else
	t1r = (t1r << 8) / sum,
        t2r = (t2r << 8) / sum;
    }

  char buffer[200];
  sprintf(buffer,"%s/t1 chrominance",T_image.q_color_scheme_label());
  t1.display(buffer);
  sprintf(buffer,"%s/t2 chrominance",T_image.q_color_scheme_label());
  t2.display(buffer);
}


/*
 *------------------------------------------------------------------------
 *			     Root modules
 */

struct MenuItem {
  char * label;
  ProcessingFunction executor;
  void * argument;
};

static MenuItem Menu [] =
{
  { "C.I.E Spectral Primary Color Coordinate System",
    convert_to_tristimulus_system,
    &(TristimulusSystem)
    { "CIE primary RGB",
      {"Monochrom Red", "Monochrom Green", "Monochrom Blue"},
      { {1167, -146, -151},
        { 114,  753,  159},
        {  -1,   59, 1128}
      },
      1000
    },
  },

  { "X-Y-Z system",
    convert_to_tristimulus_system,
    &(TristimulusSystem)
    { "XYZ", {"X", "Y - luminance", "Z"},
      { {607, 174,  201},
        {299, 587,  114},
        {  0,  66, 1117}
      },
      1000
    }
  },

  { "U-V-W Uniform Chromacity Scale",
    convert_to_tristimulus_system,
    &(TristimulusSystem)
    { "UVW", {"U", "V - luminance", "W"},
      { {405, 116,  133},
        {299, 587,  114},
        {145, 827,  627}
      },
      1000
    }
  },

  { "Y-I-Q NTSC Transmission Color Coordinate system",
    convert_to_tristimulus_system,
    &(TristimulusSystem)
    { "YIQ - NTSC TV signal", {"Y - luminance", "I-chromatic", "Q-chromatic"},
      { {299,  587,  114},
        {596, -274, -322},
        {211, -523,  312}
      },
      1000
    }
  },

  { "x-y chrominance system",
    convert_to_chrominance,
    &(TristimulusSystem)
    { "XYZ", {"X", "Y - luminance", "Z"},
      { {607, 174,  201},
        {299, 587,  114},
        {  0,  66, 1117}
      },
      1000
    }
  },

  { "u-v Uniform Chromacity Scale",
    convert_to_chrominance,
    &(TristimulusSystem)
    { "UVW", {"U", "V - luminance", "W"},
      { {405, 116,  133},
        {299, 587,  114},
        {145, 827,  627}
      },
      1000
    }
  }

};

main(const int argc, const char * argv[])
{
  cout << "\n\n\t\tDisplay the color components of the color image in various"
          "\n\t\t\t\tcolor coordinate systems\n\n";

  if( argc != 2 )
    message("\nUsage: color_coordinates image_file_in_PPM_format\n"),
    exit(4);

  {					// Display the image as it is
    char buffer[200];
    sprintf(buffer,"xv %s &",argv[1]);
    cout << "\nDisplaying the image with the command <" << buffer
         << ">\n";
    system(buffer);
  }

  RGB_image rgb_image(argv[1]);
  rgb_image.display_projections();

					// Handling menus
  for(;;)
  {
    const int no_choices = sizeof(Menu)/sizeof(MenuItem);
    register int i;

    sleep(2);
    cout << "\n\n\t\t\t\tMenu\n";		// Display the menu
    for(i=0; i<no_choices; i++)
      cout << "\t" << i+1 << ". " << Menu[i].label << "\n";


    int choice;
    for(;;)
    {
      cout << "Enter the number of your choice or 0 to quit\n";
      cin >> choice;
      if( choice == 0 )
	exit(0);
      else if( choice > 0 && choice <= no_choices )
	break;
      cout << "\nYou entered invalid number. Please enter the number "
	      "1.." << no_choices << " or 0 to quit\n";
    }
    i = choice - 1;
    Menu[i].executor(rgb_image,Menu[i].argument);
  }
}

