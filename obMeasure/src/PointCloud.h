#ifndef _POINT_CLOUD_H_
#define _POINT_CLOUD_H_

#include <vector>
#include "Point3D.h"
#include "Plane.h"

using namespace std;

class PointCloud
{
public:
	PointCloud(vector<double*> PtList);

	bool FitPlane(Plane& plane, double& std, double& min, double& max);
	bool GetMeasurePoint(Point3D& point);
	double CalcPointToPlaneDist(Point3D point, Plane plane);

private:
	bool CalcPlaneError(Plane plane, double& stdDist, double& minDist, double& maxDist);
	bool CalcCenterPoint(Point3D& centerPoint);
	
	vector<double*> points;
};

#endif