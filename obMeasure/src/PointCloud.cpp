#include "PointCloud.h"


PointCloud::PointCloud(vector<double*> PtList)
{
	points = PtList;
}


bool PointCloud::FitPlane(Plane& plane, double& stdDist, double& minDist, double& maxDist)
{
	Point3D testPoint;

	long nCount = points.size();
	if (nCount < 3)
	{
		return false;
	}

	// Ax + By + Cz + D = 0
	
	// Ax + By + C(z + dz) + D = 0
	// -C*dz = Ax + By + Cz + D
	// C^2 * dz^2 = (Ax + By + Cz + D)^2
	// M = S(dz^2) = 1/C^2 * S((Ax + By + Cz + D)^2)

	// M对A，B，C，D的偏导为0，得方程：
	// S(X2) S(XY) S(XZ) S(X)     A
	// S(XY) S(Y2) S(YZ) S(Y)     B    =   0
	// S(XZ) S(YZ) S(Z2) S(Z)     C
	// S(X)  S(Y)  S(Z)  N        D

	long double Pt[4][4];
	Pt[0][0] = 0;	Pt[0][1] = 0;	Pt[0][2] = 0;	Pt[0][3] = 0;
	Pt[1][0] = 0;	Pt[1][1] = 0;	Pt[1][2] = 0;	Pt[1][3] = 0;
	Pt[2][0] = 0;	Pt[2][1] = 0;	Pt[2][2] = 0;	Pt[2][3] = 0;
	Pt[3][0] = 0;	Pt[3][1] = 0;	Pt[3][2] = 0;	Pt[3][3] = 0;

	for (int i = 0; i < nCount; i++)
	{
		double tx, ty, tz;
		tx = *(points[i]);
		ty = *(points[i] + 1);
		tz = *(points[i] + 2);
		//printf("%.4f\t%.4f\t%.4f\n", tx, ty, tz);

		Pt[0][0] += pow(tx, 2);
		Pt[0][1] += (tx)* (ty);
		Pt[0][2] += (tx)* (tz);
		Pt[0][3] += (tx);

		Pt[1][1] += pow((ty), 2);
		Pt[1][2] += (ty)* (tz);
		Pt[1][3] += (ty);

		Pt[2][2] += pow((tz), 2);
		Pt[2][3] += (tz);

		Pt[3][3] += 1;
	}


	// 对称矩阵
	Pt[1][0] = Pt[0][1];
	Pt[2][0] = Pt[0][2];
	Pt[2][1] = Pt[1][2];
	Pt[3][0] = Pt[0][3];
	Pt[3][1] = Pt[1][3];
	Pt[3][2] = Pt[2][3];

	//	TRACE("R\n");
	//	TRACE("%.4f\t%.4f\t%.4f\t%.4f\n",Pt[0][0],Pt[0][1],Pt[0][2],Pt[0][3]);
	//	TRACE("%.4f\t%.4f\t%.4f\t%.4f\n",Pt[1][0],Pt[1][1],Pt[1][2],Pt[1][3]);
	//	TRACE("%.4f\t%.4f\t%.4f\t%.4f\n",Pt[2][0],Pt[2][1],Pt[2][2],Pt[2][3]);
	//	TRACE("%.4f\t%.4f\t%.4f\t%.4f\n",Pt[3][0],Pt[3][1],Pt[3][2],Pt[3][3]);

	// 求解，行初等变换
	for (int i = 0; i < 4; i++)
	{
		//列主元，按绝对值从大到小换行排序
		for (int j = i; j < 4; j++)
		{
			if (abs(Pt[j][i]) > abs(Pt[i][i]))
			{
				for (int t = 0; t < 4; t++)
				{
					double tmp = Pt[j][t];
					Pt[j][t] = Pt[i][t];
					Pt[i][t] = tmp;
				}
			}
		}
		//消元
		for (int j = (i + 1); j < 4; j++)
		{
			double k = Pt[j][i] / Pt[i][i];
			for (int t = 0; t < 4; t++)
			{
				Pt[j][t] = Pt[j][t] - k * Pt[i][t];
			}
		}
	}

	//	TRACE("R\n");
	//	TRACE("%.4f\t%.4f\t%.4f\t%.4f\n",Pt[0][0],Pt[0][1],Pt[0][2],Pt[0][3]);
	//	TRACE("%.4f\t%.4f\t%.4f\t%.4f\n",Pt[1][0],Pt[1][1],Pt[1][2],Pt[1][3]);
	//	TRACE("%.4f\t%.4f\t%.4f\t%.4f\n",Pt[2][0],Pt[2][1],Pt[2][2],Pt[2][3]);
	//	TRACE("%.4f\t%.4f\t%.4f\t%.4f\n",Pt[3][0],Pt[3][1],Pt[3][2],Pt[3][3]);

	double A, B, C, D;
	D = 1;
	C = -(D*Pt[2][3]) / Pt[2][2];
	B = -(D*Pt[1][3] + C*Pt[1][2]) / Pt[1][1];
	A = -(D*Pt[0][3] + C*Pt[0][2] + B*Pt[0][1]) / Pt[0][0];


	// 法向量归一化
	double tmp = sqrt(pow(A, 2) + pow(B, 2) + pow(C, 2));
	if (tmp < 0.0001 && tmp > -0.0001)
	{
		return false;
	}

	plane.A = A / tmp;
	plane.B = B / tmp;
	plane.C = C / tmp;
	
	// 根据数据所在坐标系算截距
	Point3D centerPoint;
	CalcCenterPoint(centerPoint);

	plane.D = -(plane.A * centerPoint.X + plane.B * centerPoint.Y + plane.C * centerPoint.Z);

	CalcPlaneError(plane, stdDist, minDist, maxDist);

	return true;
}

bool PointCloud::GetMeasurePoint(Point3D& point)
{
	double testx, testy, testz = 100000;
	long nCount = points.size();
	if (nCount < 3)
	{
		return false;
	}

	for (int i = 0; i < nCount; i++)
	{
		double tx, ty, tz;
		tx = *(points[i]);
		ty = *(points[i] + 1);
		tz = *(points[i] + 2);

		if (tz < testz)
		{
			testx = tx;
			testy = ty;
			testz = tz;
		}
	}

	//printf("%.4f\t%.4f\t%.4f\n", testx, testy, testz);
	point.X = testx;
	point.Y = testy;
	point.Z = testz;
	
	return true;
}

bool PointCloud::CalcPlaneError(Plane plane, double& std, double& min, double& max)
{
	
	long nCount = points.size();
	double* pDist = new double[nCount];
	long double distSum = 0;

	Point3D srcPoint;
	Point3D targetPoint;

	max = 0;
	min = 0;

	double maxDist = 0;

	for (int i = 0; i < nCount; i++)
	{
		srcPoint.X = *(points[i]);
		srcPoint.Y = *(points[i] + 1);
		srcPoint.Z = *(points[i] + 2);

		
		double dist = CalcPointToPlaneDist(srcPoint, plane);
		distSum += pow(dist, 2);

		if (dist > maxDist)
		{
			maxDist = dist;

			double planeZ = -(plane.A * srcPoint.X + plane.B * srcPoint.Y + plane.D) / plane.C;
			if (srcPoint.Z >= planeZ)
			{
				max = dist;
			}
			else
			{
				min = -dist;
			}
		}
	}

	std = sqrt(distSum / nCount);
	
	return true;
}


//      | Ax0 + By0 + Cz0 + D | 
// d =  -----------------------
//       sqrt(A^2 + B^2 + C^2)
//

double PointCloud::CalcPointToPlaneDist(Point3D point, Plane plane)
{
	double x0, y0, z0;
	x0 = point.X;
	y0 = point.Y;
	z0 = point.Z;

	double A, B, C, D;
	A = plane.A;
	B = plane.B;
	C = plane.C;
	D = plane.D;

	double d = abs(A * x0 + B * y0 + C * z0 + D) / sqrt(pow(A, 2) + pow(B, 2) + pow(C, 2));

	return d;
}



bool PointCloud::CalcCenterPoint(Point3D& centerPoint)
{
	long nCount = points.size();
	if (nCount < 1)
	{
		return false;
	}

	long double tmp[3];
	tmp[0] = 0;
	tmp[1] = 0;
	tmp[2] = 0;

	for (int i = 0; i < nCount; i++)
	{
		tmp[0] += *(points[i]);
		tmp[1] += *(points[i] + 1);
		tmp[2] += *(points[i] + 2);
	}

	centerPoint.X = tmp[0] / nCount;
	centerPoint.Y = tmp[1] / nCount;
	centerPoint.Z = tmp[2] / nCount;
	
	return true;
}