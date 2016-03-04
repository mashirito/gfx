#ifndef APPMAIN_H
#define APPMAIN_H


#include "World.h"
#include "Raytracer.h"

#include <GL/glut.h>

#define RESOLUTIONX 800
#define RESOLUTIONY 800

Raytracer g_raytracer( RESOLUTIONX , RESOLUTIONY );
World w;

#endif