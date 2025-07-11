// Persistence of Vision Raytracer 2.0
// Colors.inc
//
#declare Colors_Inc_Temp = version

#version 2.0

// Many pre-defined colors for use in scene files.
// Also includes constants for Map Types and Interpolation Types
// for use with image_map, bump_map, and material_map

//
// These grays are useful for fine-tuning lighting color values
// and for other areas where subtle variations of grays are needed.
#declare Gray05 = color rgb <0.05, 0.05, 0.05>
#declare Gray05 = color rgb <0.05, 0.05, 0.05>
#declare Gray10 = color rgb <0.10, 0.10, 0.10>
#declare Gray15 = color rgb <0.15, 0.15, 0.15>
#declare Gray20 = color rgb <0.20, 0.20, 0.20>
#declare Gray25 = color rgb <0.25, 0.25, 0.25>
#declare Gray30 = color rgb <0.30, 0.30, 0.30>
#declare Gray35 = color rgb <0.35, 0.35, 0.35>
#declare Gray40 = color rgb <0.40, 0.40, 0.40>
#declare Gray45 = color rgb <0.45, 0.45, 0.45>
#declare Gray50 = color rgb <0.50, 0.50, 0.50>
#declare Gray55 = color rgb <0.55, 0.55, 0.55>
#declare Gray60 = color rgb <0.60, 0.60, 0.60>
#declare Gray65 = color rgb <0.65, 0.65, 0.65>
#declare Gray70 = color rgb <0.70, 0.70, 0.70>
#declare Gray75 = color rgb <0.75, 0.75, 0.75>
#declare Gray80 = color rgb <0.80, 0.80, 0.80>
#declare Gray85 = color rgb <0.85, 0.85, 0.85>
#declare Gray90 = color rgb <0.90, 0.90, 0.90>
#declare Gray95 = color rgb <0.95, 0.95, 0.95>

#declare DimGray = color rgb <0.329412, 0.329412, 0.329412>
#declare DimGrey = color rgb <0.329412, 0.329412, 0.329412>
#declare Gray = color rgb <0.752941, 0.752941, 0.752941>
#declare Grey = color rgb <0.752941, 0.752941, 0.752941>
#declare LightGray = color rgb <0.658824, 0.658824, 0.658824>
#declare LightGrey = color rgb <0.658824, 0.658824, 0.658824>
#declare VLightGrey = color rgb <0.8, 0.8, 0.80>

#declare Clear = color rgbf <1, 1, 1, 1>
#declare White = color rgb <1, 1, 1>
#declare Red   = color red 1
#declare Green = color green 1
#declare Blue  = color blue 1
#declare Yellow  = color red 1 green 1
#declare Cyan    = color green 1 blue 1
#declare Magenta = color red 1 blue 1
#declare Black   = color rgb <0, 0, 0>
#declare Aquamarine = color rgb <0.439216, 0.858824, 0.576471>
#declare BlueViolet = color rgb <0.62352, 0.372549, 0.623529>
#declare Brown      = color rgb <0.647059, 0.164706, 0.164706>
#declare CadetBlue  = color rgb <0.372549, 0.623529, 0.623529>
#declare Coral      = color rgb <1, 0.498039, 0>
#declare CornflowerBlue = color rgb <0.258824, 0.258824, 0.435294>
#declare DarkGreen      = color rgb <0.184314, 0.309804, 0.184314>
#declare DarkOliveGreen = color rgb <0.309804, 0.309804, 0.184314>
#declare DarkOrchid    = color rgb <0.6, 0.196078, 0.8>
#declare DarkSlateBlue = color rgb <0.419608, 0.137255, 0.556863>
#declare DarkSlateGray = color rgb <0.184314, 0.309804, 0.309804>
#declare DarkSlateGrey = color rgb <0.184314, 0.309804, 0.309804>
#declare DarkTurquoise = color rgb <0.439216, 0.576471, 0.858824>
#declare Firebrick   = color rgb <0.556863, 0.137255, 0.137255>
#declare ForestGreen = color rgb <0.137255, 0.556863, 0.137255>
#declare Gold        = color rgb <0.8, 0.498039, 0.196078>
#declare Goldenrod   = color rgb <0.858824, 0.858824, 0.439216>
#declare GreenYellow = color rgb <0.576471, 0.858824, 0.439216>
#declare IndianRed   = color rgb <0.309804, 0.184314, 0.184314>
#declare Khaki     = color rgb <0.623529, 0.623529, 0.372549>
#declare LightBlue = color rgb <0.74902, 0.847059, 0.847059>
#declare LightSteelBlue = color rgb <0.560784, 0.560784, 0.737255>
#declare LimeGreen = color rgb <0.196078, 0.8, 0.196078>
#declare Maroon    = color rgb <0.556863, 0.137255, 0.419608>
#declare MediumAquamarine  = color rgb <0.196078, 0.8, 0.6>
#declare MediumBlue        = color rgb <0.196078, 0.196078, 0.8>
#declare MediumForestGreen = color rgb <0.419608, 0.556863, 0.137255>
#declare MediumGoldenrod = color rgb <0.917647, 0.917647, 0.678431>
#declare MediumOrchid    = color rgb <0.576471, 0.439216, 0.858824>
#declare MediumSeaGreen  = color rgb <0.258824, 0.435294, 0.258824>
#declare MediumSlateBlue = color red 0.498039 blue 1
#declare MediumSpringGreen = color red 0.498039 green 1
#declare MediumTurquoise = color rgb <0.439216, 0.858824, 0.858824>
#declare MediumVioletRed = color rgb <0.858824, 0.439216, 0.576471>
#declare MidnightBlue = color rgb <0.184314, 0.184314, 0.309804>
#declare Navy     = color rgb <0.137255, 0.137255, 0.556863>
#declare NavyBlue = color rgb <0.137255, 0.137255, 0.556863>
#declare Orange   = color rgb <1, 0.5, 0>
#declare OrangeRed = color red 1 blue 0.498039
#declare Orchid    = color rgb <0.858824, 0.439216, 0.858824>
#declare PaleGreen = color rgb <0.560784, 0.737255, 0.560784>
#declare Pink = color rgb <0.737255, 0.560784, 0.560784>
#declare Plum = color rgb <0.917647, 0.678431, 0.917647>
#declare Salmon   = color rgb <0.435294, 0.258824, 0.258824>
#declare SeaGreen = color rgb <0.137255, 0.556863, 0.419608>
#declare Sienna   = color rgb <0.556863, 0.419608, 0.137255>
#declare SkyBlue  = color rgb <0.196078, 0.6, 0.8>
#declare SlateBlue = color rgb <0, 0.498039, 1>
#declare SpringGreen = color rgb <0, 1, 0.498039>
#declare SteelBlue = color rgb <0.137255, 0.419608, 0.556863>
#declare Tan     = color rgb <0.858824, 0.576471, 0.439216>
#declare Thistle = color rgb <0.847059, 0.74902, 0.847059>
#declare Turquoise = color rgb <0.678431, 0.917647, 0.917647>
#declare Violet    = color rgb <0.309804, 0.184314, 0.309804>
#declare VioletRed = color rgb <0.8, 0.196078, 0.6>
#declare Wheat     = color rgb <0.847059, 0.847059, 0.74902>
#declare YellowGreen = color rgb <0.6, 0.8, 0.196078>
#declare SummerSky = color rgb <0.22, 0.69, 0.87>
#declare RichBlue  = color rgb <0.35, 0.35, 0.67>
#declare Brass   = color rgb <0.71, 0.65, 0.26>
#declare Copper  = color rgb <0.72, 0.45, 0.20>
#declare Bronze  = color rgb <0.55, 0.47, 0.14>
#declare Bronze2 = color rgb <0.65, 0.49, 0.24>
#declare Silver  = color rgb <0.90, 0.91, 0.98>
#declare BrightGold = color rgb <0.85, 0.85, 0.10>
#declare OldGold    = color rgb <0.81, 0.71, 0.23>
#declare Feldspar   = color rgb <0.82, 0.57, 0.46>
#declare Quartz     = color rgb <0.85, 0.85, 0.95>
#declare Mica       = color Black  
#declare NeonPink   = color rgb <1.00, 0.43, 0.78>
#declare DarkPurple = color rgb <0.53, 0.12, 0.47>
#declare NeonBlue   = color rgb <0.30, 0.30, 1.00>
#declare CoolCopper = color rgb <0.85, 0.53, 0.10>
#declare MandarinOrange = color rgb <0.89, 0.47, 0.20>
#declare LightWood  = color rgb <0.91, 0.76, 0.65>
#declare MediumWood = color rgb <0.65, 0.50, 0.39>
#declare DarkWood   = color rgb <0.52, 0.37, 0.26>
#declare SpicyPink  = color rgb <1.00, 0.11, 0.68>
#declare SemiSweetChoc = color rgb <0.42, 0.26, 0.15>
#declare BakersChoc = color rgb <0.36, 0.20, 0.09>
#declare Flesh  = color rgb <0.96, 0.80, 0.69>
#declare NewTan = color rgb <0.92, 0.78, 0.62>
#declare NewMidnightBlue = color rgb <0.00, 0.00, 0.61>
#declare VeryDarkBrown   = color rgb <0.35, 0.16, 0.14>
#declare DarkBrown = color rgb <0.36, 0.25, 0.20>
#declare DarkTan   = color rgb <0.59, 0.41, 0.31>
#declare GreenCopper   = color rgb <0.32, 0.49, 0.46>
#declare DkGreenCopper = color rgb <0.29, 0.46, 0.43>
#declare DustyRose     = color rgb <0.52, 0.39, 0.39>
#declare HuntersGreen  = color rgb <0.13, 0.37, 0.31>
#declare Scarlet = color rgb <0.55, 0.09, 0.09>

// Map types constants
// for use with image_map, bump_map, and material_map
//
// Format:
//   map_type Sphere_Map or map_type Torus_Map
//
#declare Plane_Map = 0
#declare Sphere_Map = 1
#declare Cylinder_Map = 2
#declare Torus_Map = 5

// Interpolation constants
// Use in image_map and bump_map in form:
//
// interpolate Bi or interpolate Norm
//
#declare Bi   = 2    // Bilinear interpolation is best
#declare Norm = 4    // Normalized distance is a bit faster

#version Colors_Inc_Temp
