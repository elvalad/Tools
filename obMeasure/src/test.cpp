/* +---------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)               |
   |                          http://www.mrpt.org/                             |
   |                                                                           |
   | Copyright (c) 2005-2016, Individual contributors, see AUTHORS file        |
   | See: http://www.mrpt.org/Authors - All rights reserved.                   |
   | Released under BSD License. See details in http://www.mrpt.org/License    |
   +---------------------------------------------------------------------------+ */

#include "Sensor.h"
#include "Display.h"
#include "Plane.h"
#include "PointCloud.h"

#include <mrpt/gui/CDisplayWindow3D.h>
#include <mrpt/random.h>
#include <mrpt/utils/CTicTac.h>
#include <mrpt/poses/CPose3D.h>
#include <mrpt/opengl/CGridPlaneXY.h>
#include <mrpt/opengl/CPointCloud.h>
#include <mrpt/opengl/stock_objects.h>
#include <mrpt/opengl/CTexturedPlane.h>

#include <mrpt/gui.h>
#include <mrpt/opengl.h>
#include <mrpt/maps/CColouredPointsMap.h>
#include <mrpt/system/threads.h>

#include <OpenNI.h>


using namespace mrpt;
using namespace mrpt::maps;
using namespace mrpt::utils;
using namespace mrpt::gui;
using namespace mrpt::math;
using namespace mrpt::random;
using namespace mrpt::poses;
using namespace std;

#define PI 3.141592653
#define DEGREETORAD(X) ((X)*PI/180)

CColouredPointsMap framePoints;
int roiX = 0;
int roiY = 0;
int roiW = 200;
int roiH = 200;
int roiStep = 10;

float depth_HFOV = 58.4;
float depth_VFOV = 45.5;
float color_HFOV = 63.1;
float color_VFOV = 49.4;

bool registered;
double depthPointCloud[RES_WIDTH / 2 * RES_HEIGHT / 2 * 3];
int numOfSamples = 0;

Plane oldPlane;
Point3D newPoint;
bool bMeasure = false;
bool bDebug = false;

// For test
void GeneratePlane(int width, int height)
{
	framePoints.clear();
	randomGenerator.randomize();

	int k = 0; 
	numOfSamples = 0;

	const double PLANE_EQ[4] = { 1, -1, 1, -2 }; 
	//const double PLANE_EQ[4] = { 0, 0, 1, -0.724 };
	// Ax + By + Cz + D = 0
	// z = -(D + Ax + By) / C
	// A = EQ[0], B = EQ[1], C = EQ[2], D = EQ[3]


	for (int yc = 0; yc < height; ++yc)
	{
		for (int xc = 0; xc < width; ++xc)
		{
			double x = 2.0 * (xc - width / 2.0) / width; //randomGenerator.drawUniform(-1, 1);
			double y = 2.0 * (yc - height / 2.0) / height; //randomGenerator.drawUniform(-1, 1);
			double z = -(PLANE_EQ[3] + PLANE_EQ[0] * x + PLANE_EQ[1] * y) / PLANE_EQ[2];
			z += randomGenerator.drawUniform(-0.010, 0.010);

			// color
			float r = 0.0039 * 128;
			float g = 0.0039 * 128;
			float b = 0.0039 * 128;

			// set ROI
			if (xc >= roiX && xc < (roiX + roiW) && yc >= roiY && yc < (roiY + roiH))
			{

				if ((xc - roiX) % roiStep == 0 && (yc - roiY) % roiStep == 0)
				{
					//if (z > 0)
					{
						numOfSamples++;

						depthPointCloud[k] = x;
						depthPointCloud[k + 1] = y;
						depthPointCloud[k + 2] = z;
						k += 3;

						g = 0;
						b = 0;
					}
				}
				else
				{
					b = 0;
				}
			}

			framePoints.insertPoint(x, y, z, r, g, b);
		}
	}
}


void GetPointCloud(int width, int height, const openni::DepthPixel* depthBuffer, const openni::RGB888Pixel* colorBuffer, int rowSize)
{

	framePoints.clear();


	float x, y, z;

	const openni::DepthPixel* pDepthRow = depthBuffer;
	const openni::RGB888Pixel* pRgbRow = colorBuffer;

	int i = 0;
	int k = 0;
	numOfSamples = 0;

	for (int yc = 0; yc < height; ++yc)
	{
		const openni::DepthPixel* pDepth = pDepthRow;
		const openni::RGB888Pixel* pRgb = pRgbRow;

		for (int xc = 0; xc < width; ++xc, ++pDepth, ++pRgb)
		{
			// depth
			z = 0.001*(*pDepth);

			// X = (x / resX - 0.5) * tan(FOV_h / 2) * 2 * d
			// Y = (y / resY - 0.5) * tan(FOV_v / 2) * 2 * d

			if (registered)
			{
				x = (0.5 - 1.0 * xc / width) * tan(DEGREETORAD(color_HFOV / 2)) * 2 * z;
				y = (0.5 - 1.0 * yc / height) * tan(DEGREETORAD(color_VFOV / 2)) * 2 * z;
			}
			else
			{
				x = (0.5 - 1.0 * xc / width) * tan(DEGREETORAD(depth_HFOV / 2)) * 2 * z;
				y = (0.5 - 1.0 * yc / height) * tan(DEGREETORAD(depth_VFOV / 2)) * 2 * z;
			}

			// color
			float r = 0.0039 * pRgb->r;
			float g = 0.0039 * pRgb->g;
			float b = 0.0039 * pRgb->b;

			// set ROI
			if (xc >= roiX && xc < (roiX + roiW) && yc >= roiY && yc < (roiY + roiH))
			{
				if ((xc - roiX) % roiStep == 0 && (yc - roiY) % roiStep == 0)
				{
					if (z > 0)
					{
						numOfSamples++;

						depthPointCloud[k] = x;
						depthPointCloud[k + 1] = y;
						depthPointCloud[k + 2] = z;
						k += 3;

						if (bDebug)
						{
							r = 0.0039 * 128;
							g = 0;
							b = 0;
						}
					}
				}
				else
				{
					if (bDebug)
					{
						r = 0.0039 * 128;
						g = r;
						b = 0;
					}
				}
			}


			framePoints.insertPoint(x, y, z, r, g, b);
		}

		pDepthRow += rowSize;
		pRgbRow += rowSize;
	}

	//cout << "num of samples " << numOfSamples << endl;
}


Sensor sensor; // Use global variable to avoid stack overflow

double stdDist, minDist, maxDist;

void Fit(double* pData, int size, Plane& plane, Point3D& point, double& distance)
{
	int k = 0;
	vector <double *> pointList; // List of data address

	// Prepare point cloud
	double max = 0.0;
	for (int i = 0; i < size; i++)
	{
		pointList.push_back(pData + k);
		k += 3;
	}
	
	PointCloud pcl(pointList);

	Plane tmpPlane;
	pcl.FitPlane(tmpPlane, stdDist, minDist, maxDist);
	pcl.GetMeasurePoint(point);

	plane.A = tmpPlane.A;
	plane.B = tmpPlane.B;
	plane.C = tmpPlane.C;
	plane.D = tmpPlane.D;

	if (!bMeasure)
	{
		oldPlane.A = plane.A;
		oldPlane.B = plane.B;
		oldPlane.C = plane.C;
		oldPlane.D = plane.D;
	}
	
	distance = pcl.CalcPointToPlaneDist(point, oldPlane);
}

void RunTest()
{

	Display display;
	display.InitScene();

	Plane plane;

	sensor.Start();
	
	registered = sensor.getRegistration();

	cout << "ROI(" << roiX << " " << roiY << " " << roiW << " " << roiH << ")" << endl;
	cout << "depth FOV (" << depth_HFOV << " " << depth_VFOV << ")\ncolor FOV (" << color_HFOV << " " << color_VFOV << ")" << endl;

	int msgDrawInterval = 15;
	int msgDrawIndex = 0;

	while (true)
	{
		// Response to key
		if (display.window.keyHit())
		{
			int key = display.window.getPushedKey();
			//cout << "key " << key << endl;
			// ESC
			if (key == 27)
			{
				break;
			}

			// r
			if (key == 114)
			{
				sensor.toggleRegistration();
			}

			// c
			if (key == 99)
			{
				bMeasure = !bMeasure;
				if (bMeasure)
				{
					roiX = 160;
					roiY = 120;
					roiW = 320;
					roiH = 240;
				}
				else
				{
					roiX = 0;
					roiY = 0;
					roiW = 200;
					roiH = 200;
				}
			}

			// d
			if (key == 100)
			{
				bDebug = !bDebug;
			}
		}

		sensor.ReadFrame();

		GetPointCloud(RES_WIDTH, RES_HEIGHT, sensor.depthBuffer, sensor.colorBuffer, sensor.rowSize);
		//GeneratePlane(RES_WIDTH, RES_HEIGHT);

		float centerX, centerY, centerZ;
		double distance;
	    framePoints.getPoint(RES_WIDTH * RES_HEIGHT / 2 + RES_WIDTH / 2, centerX, centerY, centerZ);
		Fit(depthPointCloud, numOfSamples, plane, newPoint, distance);

		display.DrawFrame(&framePoints);
		display.DrawPlane(display.glPlane, plane.A, plane.B, plane.C, plane.D);

		msgDrawIndex++;
		if (msgDrawIndex >= msgDrawInterval)
		{
			msgDrawIndex = 0;

			//printf("%.4f\t%.4f\t%.4f\n", newPoint.X, newPoint.Y, newPoint.Z);
			display.DrawMessage(format("Measure distance %.3fm\n", distance));
			//display.DrawMessage(format("Measure dist %.3f Registration %d\nPlane A %.3f B %.3f C %.3f D %.3f\n", 
				//distance, sensor.getRegistration(), plane.A, plane.B, plane.C, plane.D));
			//display.DrawMessage(format("Dist %.3f Registration %d\nPlane A %.3f B %.3f C %.3f D %.3f \nError std %.3f min %.3f max %.3f",
				//centerZ, sensor.getRegistration(), plane.A, plane.B, plane.C, plane.D, stdDist, minDist, maxDist));
		}
	} 

	sensor.Stop();
}


int main()
{
	try
	{
		RunTest();
		return 0;
	} catch (std::exception &e)
	{
		std::cout << "MRPT exception caught: " << e.what() << std::endl;
		return -1;
	}
	catch (...)
	{
		printf("Untyped exception!!");
		return -1;
	}
}