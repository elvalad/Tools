#pragma once

#include "opencv2/core/core.hpp"
#include "SoftwareRegistrator.h"

#define MAX_DEPTH_WIDTH  1280
#define MAX_DEPTH_HEIGHT 1024


class PointCloud
{
public:
	int update(cv::Mat depthMap, cv::Mat colorMap, bool useDistortCoef);
	int save(const char* fileName);
	SoftwareRegistrator* pSoftReg;

private:

	float vertex[MAX_DEPTH_HEIGHT][MAX_DEPTH_WIDTH][3];
	float texture[MAX_DEPTH_HEIGHT][MAX_DEPTH_WIDTH][3];

	int mWidth;
	int mHeight;
};
