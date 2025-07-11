//#include <MacHeadersPPC> // MAART
#include "Scene3D.h"
#include "Object3D.h"
#include "Vector3D.hpp"
#include "View3D.hpp"
#include "Light3D.h"

#include <stdio.h>
#include <stdlib.h>

#if __profile__
#include <profiler.h>
#endif



extern long polygonsDrawn;

void InitToolbox(void);
void benchmark(void);

void InitToolbox()
{
	InitGraf((Ptr) &qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	FlushEvents(everyEvent,0);
	TEInit();
	InitDialogs(0L);
	InitCursor();
}


void benchmark()
{

  //  Create a new empty scene
  Scene3D scene;
  scene.SetAmbientIntensity(0.10);

#if 1
  // Add the x axis
  Object3D xaxis;
  Vector3D cylinderTop(5., 0., 0.);
  Vector3D cylinderBottom(0., 0., 0.);
  xaxis.AddCylinder(&cylinderTop, &cylinderBottom, TRUE, TRUE, 0.5);
  Vector3D coneTop(6., 0., 0.);
  Vector3D coneBottom(5., 0., 0.);
  xaxis.AddCone(&coneTop, &coneBottom, TRUE, TRUE, 0, 1.0);
  xaxis.Triangulate();
  xaxis.SetShadingMethod(shadeWireframe);
  scene.AddObject(&xaxis);

  //  Add the y axis
  Object3D yaxis;
  cylinderTop.SetDouble(0., 5., 0.);
  cylinderBottom.SetDouble(0., 0., 0.);
  yaxis.AddCylinder(&cylinderTop, &cylinderBottom, TRUE, TRUE, 0.5);
  coneBottom.SetDouble(0., 5., 0.);
  yaxis.AddSphere(&coneBottom, 1.); //, TRUE, TRUE, 0, 1.0);
  yaxis.Triangulate();
  yaxis.SetShadingMethod(shadeFaceted);
  scene.AddObject(&yaxis);

  //  Add the z axis
  Object3D zaxis;
  cylinderTop.SetDouble(0, 0, 4);
  cylinderBottom.SetDouble(0, 0, 0);
  zaxis.AddCylinder(&cylinderTop, &cylinderBottom, TRUE, TRUE, 0.5);
  coneTop.SetDouble(0, 0, 6);
  coneBottom.SetDouble(0, 0, 5);
  zaxis.AddCone(&coneTop, &coneBottom, TRUE, TRUE, 0, 1.0);
  coneTop.SetDouble(0, 0, 5);
  coneBottom.SetDouble(0, 0, 4);
  zaxis.AddCone(&coneTop, &coneBottom, TRUE, TRUE, 0, 1.0);
  zaxis.Triangulate();
  zaxis.SetShadingMethod(shadeSmooth);
  scene.AddObject(&zaxis);
#else
  //  Add a test object
  Object3D sphere;
  Vector3D point0 (0., 0., 0.);
  sphere.AddSphere(&point0, 1.);
  sphere.SetShadingMethod(shadeFaceted);
  scene.AddObject(&sphere);
  Object3D test;
  Vector3D point1 (0., 0., 1.);
  Vector3D point2 (0., 0., -1.);
  Vector3D point3 (0., 1., 0.);
  test.AddTriangle(&point1, &point2, &point3);
  test.SetShadingMethod(shadeWireframe);
  scene.AddObject(&test);
#endif
  
  //  Create two lights, and add them to the scene
  Vector3D lightPos(10., 10., 10.);
  Color3D lightColor(1., 1., 1.);
  Light3D light(lightPos, lightColor);
  scene.AddLight(&light);
  Vector3D light2Pos(-10.0, -10.0, -10.0);
  Color3D light2Color(0., 0., 1.);
  Light3D light2(light2Pos, light2Color);
  scene.AddLight(&light2);

  // Circle viewpoint around object
  double t;
  double radius = 10;
  View3D *view;
  view = new View3D(&scene, 640, 400);
  Vector3D viewpoint;
  Vector3D aimpoint(0., 2., 0.);
  view->SetAimPoint(aimpoint);
  
  while (1)
    {
    for (t = -3.14; t < 3.14; t += 0.10)
	  {
	  if (Button())
	    return;
      viewpoint.SetDouble(radius*sin(t), 2., radius*cos(t));
      //viewpoint.SetDouble(5, 0, 0);
      view->SetViewpoint(viewpoint);
      view->Render();	    
      }
    }

  delete view;

}

void main()
{
#if __profile__
  if (ProfilerInit(collectDetailed, bestTimeBase, 100, 100) == noErr) {
#endif

  //  Initialize the Macintosh Toolbox
  InitToolbox();
  
#ifdef TEST_MEMORY
  // DEBUG memory allocation
  int   i, j;
  Real *ptr;
  size_t numBytes;
  long numReals;
  while (!Button()) {
    numReals   = 640 * 480;
    numBytes   = numReals;
    numBytes  *= sizeof(Real);
    
    ptr = (Real *) NewPtr((Size) numBytes);
    if (ptr != NULL) {
    	for (j = 0; j < numReals; j++) {
    		ptr[j] = DOUBLE_TO_REAL(1.0);
    	}
    }
    else {
    	SysBeep(1);
    }
    DisposPtr((Ptr) ptr);
    
    ptr = (Real *) malloc(numBytes);
    if (ptr != NULL) {
    	for (j = 0; j < numReals; j++) {
    		ptr[j] = DOUBLE_TO_REAL(1.0);
    	}
    }
    else {
    	SysBeep(1);
    }
    free((void *) ptr); 
    
    ptr = new Real [numReals];
    if (ptr != NULL) {
    	for (j = 0; j < numReals; j++) {
    		ptr[j] = DOUBLE_TO_REAL(1.0);
    	}
    }
    else {
    	SysBeep(1);
    }
    delete [] ptr;
  }
#endif

  benchmark();
#if __profile__
    ProfilerDump("\pGraph3D.prof");
    ProfilerTerm();
  }
#endif

}






