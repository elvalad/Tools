#include <iostream>
#include <fstream>
#include <omp.h>

#include "PointCloud.h"

#define PI  3.14159265358979323846


using namespace std;
using namespace cv;

int PointCloud::update(Mat depthMap, Mat colorMap, bool useDistortCoef)
{
	int depthW = depthMap.cols;
	int depthH = depthMap.rows;

	int colorW = colorMap.cols;
	int colorH = colorMap.rows;

	// TODO: support different resolution between depth and color
	if (depthW != colorW || depthH != colorH)
	{
		return -1;
	}

	mWidth = colorW;
	mHeight = colorH;

	// update vertex
	#pragma omp parallel for
	for (int v = 0; v < depthH; v++)
	{
		unsigned short* pDepth = (unsigned short*)depthMap.ptr<uchar>(v);

		for (int u = 0; u < depthW; u++)
		{
			int z = *pDepth++;
			float px = 0;
			float py = 0;
			float pz = 0;
			if (pSoftReg)
			{
				pSoftReg->ConvertProjectiveToWorld(u, v, z, px, py, pz, useDistortCoef);
			}

			vertex[v][u][0] = px;
			vertex[v][u][1] = py;
			vertex[v][u][2] = (float)z;
		}
	}

	// update texture
	#pragma omp parallel for
	for (int v = 0; v < colorH; v++)
	{
		Vec3b *pColor = colorMap.ptr<Vec3b>(v);
		for (int u = 0; u < colorW; u++)
		{
			texture[v][u][0] = pColor[u][0];
			texture[v][u][1] = pColor[u][1];
			texture[v][u][2] = pColor[u][2];
		}
	}

	return 0;
}


int PointCloud::save(const char* fileName)
{

	ofstream ouF;
	ouF.open(fileName, ofstream::out);
	if (!ouF)
	{
		cerr << "failed to open the file : " << fileName << endl;
		return -1;
	}

	ouF << "ply" << std::endl;
	ouF << "format ascii 1.0" << std::endl;
	ouF << "comment made by Orbbec " << std::endl;
	ouF << "comment Orbbec Co.,Ltd." << std::endl;
	ouF << "element vertex " << mWidth * mHeight << std::endl;
	ouF << "property float32 x" << std::endl;
	ouF << "property float32 y" << std::endl;
	ouF << "property float32 z" << std::endl;
	ouF << "property uint8 red" << std::endl;
	ouF << "property uint8 green" << std::endl;
	ouF << "property uint8 blue" << std::endl;
	ouF << "element face 0" << std::endl;
	ouF << "property list uint8 int32 vertex_index" << std::endl;
	ouF << "end_header" << std::endl;


	for (int v = 0; v < mHeight; v++)
	{
		for (int u = 0; u < mWidth; u++)
		{
			float x = vertex[v][u][0];
			float y = vertex[v][u][1];
			float z = vertex[v][u][2];

			float r = texture[v][u][0];
			float g = texture[v][u][1];
			float b = texture[v][u][2];

			ouF << x << " " << y << " " << z << " " << r << " " << g << " " << b << std::endl;
		}
	}

	ouF.close();

	return 0;
}