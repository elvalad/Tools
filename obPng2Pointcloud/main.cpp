
#include <iostream>
#include <string>
#include <math.h>
#include <stdint.h>
#include <fstream>
#include <vector>

// OpenNI2
#include "OpenNI.h" 

// OpenCV 
#include "opencv/highgui.h"
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"

#include "File.h"
#include "SoftwareRegistrator.h"

#pragma warning(disable:4996)

using namespace std;
using namespace cv;
using namespace openni;

#define CAMERA_PARAMS_INI   "./camera_params.ini"
#define DEPTH_FILES_INI     "./depth_files.ini"

SoftwareRegistrator softwareRegistrator;
vector<string> fileNameVector;
vector<int> fileConfigVector;
int fileSize;

void LoadDepthFiles(char * uri)
{
	char tmpbuf[128];
	int tmp;
	std::ifstream inf;

	inf.open(uri, std::ios::in);

	inf >> fileSize;
	for (int i = 0; i < fileSize; i++)
	{
		inf >> tmpbuf >> tmp;
		fileNameVector.push_back(tmpbuf);
		fileConfigVector.push_back(tmp);
	}
}

int main(int argc, char** argv)
{
	cout << "********** depth 2 pointcloud **********" << endl;

	// Load config
	int REC_WIDTH = 640;
	int REC_HEIGHT = 480;

	Mat     cvRGBImg(REC_HEIGHT, REC_WIDTH, CV_8UC3);
	Mat     cvDepthImg(REC_HEIGHT, REC_WIDTH, CV_16UC1);
	Mat     CaliDepth(REC_HEIGHT, REC_WIDTH, CV_16UC1);

	if (File::Exist(CAMERA_PARAMS_INI))
	{
		softwareRegistrator.LoadParamsFromIniFile(CAMERA_PARAMS_INI);
		softwareRegistrator.CalcConversionMatrix();
	}
	else
	{
		cout << "Param file not found!" << endl;
	}

	LoadDepthFiles(DEPTH_FILES_INI);

	for (int i = 0; i < fileSize; i++)
	{
		string pointcloudName = fileNameVector.at(i) + ".ply";
		cout << "Convert " << fileNameVector.at(i) << " " << fileConfigVector.at(i) << endl;
		cvDepthImg = cv::imread(fileNameVector.at(i) + ".png", CV_LOAD_IMAGE_UNCHANGED);
		if (fileConfigVector.at(i) == 1)
		{
			softwareRegistrator.MappingDepth2Color(cvDepthImg, CaliDepth, false, true, pointcloudName.c_str());
		}
		else if (fileConfigVector.at(i) == 0)
		{
			softwareRegistrator.MappingDepth2Color(cvDepthImg, CaliDepth, false, false, pointcloudName.c_str());
		}
	}

	system("pause");
	return 0;
}
