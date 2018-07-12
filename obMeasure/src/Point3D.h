#ifndef _POINT3D_H_
#define _POINT3D_H_

class Point3D
{
public:
	Point3D(void){}

	Point3D(double x, double y, double z)
	{
		X = x;
		Y = y;
		Z = z;
	}

	double X, Y, Z;
};

#endif