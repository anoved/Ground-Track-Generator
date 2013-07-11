/*
 * solarposition
 * 
 * This program prints the current julian date, the current greenwich sidereal
 * time (earth rotation in radians), and current location of the sun, expressed
 * in Earth-centered inertial coordinate system kilometers. (w is direct radius)
 * It is intended as a reference for other ECI solar position implementations.
 */

#include <stdio.h>
#include "DateTime.h"
#include "SolarPosition.h"
#include "Vector.h"

int main(void) {
	SolarPosition sp;
	DateTime date = DateTime::Now();
	Eci solar_eci = sp.FindPosition(date);
	Vector pos_eci = solar_eci.Position();
	double julianDate = date.ToJulian();
	double gsidereal = date.ToGreenwichSiderealTime();
	
	printf("d: %lf\n", julianDate);
	printf("g: %lf\n", gsidereal);
	printf("x: %lf\n", pos_eci.x);
	printf("y: %lf\n", pos_eci.y);
	printf("z: %lf\n", pos_eci.z);
	printf("w: %lf\n", pos_eci.w);
	
	return 0;
}
