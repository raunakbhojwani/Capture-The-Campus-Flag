/*	latlongdist.c	accurately calculates the lat and long using the Haversine formula 
	
	Project name: Project Incomputable
	Component name: lib/latlongdist.c
	
	Primary Author:	Max Zhuang, Raunak Bhojwani, Samuel Ching
	Date Created: 5/27/16

	Acknowledgements: Referenced http://www.movable-type.co.uk/scripts/latlong.html and http://andrew.hedges.name/experiments/haversine/
	
======================================================================*/
// do not remove any of these sections, even if they are empty
//
// ---------------- Open Issues 

// ---------------- System includes e.g., <stdio.h>
#include <stdio.h>
#include <math.h>

// ---------------- Local includes  e.g., "file.h"

// ---------------- Constant definitions 

const float pi = 3.141592653589793238462643383279;

// ---------------- Macro definitions

// ---------------- Structures/Types 

// ---------------- Private variables 

// ---------------- Private prototypes 
static double to_rad(float degs);

/*====================================================================*/

double distance(float lat1, float long1, float lat2, float long2)
{
	double radius = 6373; //taken from http://andrew.hedges.name/experiments/haversine/
	double lat1rad = to_rad(lat1);
	double lat2rad = to_rad(lat2);
	double latDiffRad = to_rad(lat2-lat1);
	double longDiffRad = to_rad(long2 - long1);

	double a = sin(latDiffRad/2) * sin(latDiffRad/2) + cos(lat1rad) * cos(lat2rad) * sin(longDiffRad) * sin(longDiffRad);
	double c = 2 * atan2(sqrt(a), sqrt(1-a));
	
	double dist = radius * c * 1000; //convert it to meters

	return dist;
}

//Helper Function

static double to_rad(float degs)
{	
	return (degs * pi/180);
}
