

#include <iostream>
#include <string>
#include <math.h>

// OpenNI2
#include "OpenNI.h" 

// OpenCV 
#include "opencv/highgui.h"
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"

#include "RGBDCamera.h"
#include "SoftwareRegistrator.h"
#include "Draw.h"
#include "Key.h"
#include "File.h"
#include "PointCloud.h"

#pragma warning(disable:4996)

using namespace std;
using namespace cv;
using namespace openni;

#define CONFIG_INI          "./config.ini"
#define CAMERA_PARAMS_INI   "./camera_params.ini"


char* strRegisterInfo = "";
char* strUseDistortion = "";
char strRegisterType[100] = "";
char strRegisterError[100] = "";
SoftwareRegistrator softwareRegistrator;
PointCloud pointCloud; // note: don't push this object in stack
int lastRegistrationType = -1;


void SwitchRegistrationType(int& type, CRGBDCamera& camera)
{

	if (lastRegistrationType == type)
	{
		return;
	}

	bool success = true;

	switch (type)
	{
	case 1:
		camera.toggleRegister(false);

		sprintf(strRegisterType, "");
		strRegisterInfo = "Registration Off";
		break;

	case 2:
		camera.toggleRegister(true);

		sprintf(strRegisterType, "[D2C]");
		strRegisterInfo = "Hardware Registration";
		break;

	case 3:

		if (File::Exist(CAMERA_PARAMS_INI))
		{
			camera.toggleRegister(false);

			softwareRegistrator.LoadParamsFromIniFile(CAMERA_PARAMS_INI);
			softwareRegistrator.CalcConversionMatrix();

			sprintf(strRegisterType, "[D2C]");
			strRegisterInfo = "Software Registration(load params from file)";

		}
		else
		{
			success = false;
			cout << "Param file not found!" << endl;
		}

		break;

	case 4:
		OBCameraParams params;
		if (camera.LoadCameraParams(params) >= 0)
		{
			camera.toggleRegister(false);

			softwareRegistrator.SetParams(params);
			softwareRegistrator.CalcConversionMatrix();

			sprintf(strRegisterType, "[D2C]");
			strRegisterInfo = "Software Registration(load params from flash)";

		}
		else
		{
			success = false;
			cout << "Error loading parameters from flash!" << endl;
		}

		break;

	case 5:
		camera.toggleRegister(false);

		sprintf(strRegisterType, "[C2D]");
		break;

	default:
		cout << "Not supported registration option!" << endl;
		break;
	}

	if (success)
	{
		lastRegistrationType = type;
	}
	else
	{
		type = lastRegistrationType; // reset to last type
	}
	
}


int main(int argc, char** argv)
{
    cout << " *********** Astra Viewer v1.6*********" << endl << endl;
    cout << " 1 - Registration Off" << endl; 
    cout << " 2 - [D2C] Hardware Registration" << endl;
    cout << " 3 - [D2C] Software Registration(load params from file)" << endl;
    cout << " 4 - [D2C] Software Registration(load params from flash)" << endl;
    cout << " 5 - [C2D] (Press '3' or '4' first)" << endl;
	cout << " 6 - [D2C] Use distortion parameters" << endl;
    cout << " O - Switch RGBD Overlay" << endl;
    cout << " C - Increase the Color proportion (RGBD overlay only)" << endl;
    cout << " c - Decrease the Color proportion (RGBD overlay only)" << endl;
	cout << " P - Save Point Cloud" << endl;
    cout << " Esc - Exit" << endl << endl;

    char * _winTitle = "Astra Viewer";

	const char* uri = NULL;
	bool readDepthFromFile = false;

	// Parse commands
	for (int i = 1; i < argc; ++i)
	{
		if (0 == strcmp(argv[i], "-file"))
		{
			readDepthFromFile = true;
		}
		else
		{
			uri = argv[i];
		}
	}

    Draw mDraw;

	// Load config
	int REC_WIDTH = 640;
	int REC_HEIGHT = 480;
	float DEPTH_FOV_H = 58.4f;
	float DEPTH_FOV_V = 45.5f;

	bool configExist = File::Exist(CONFIG_INI);
    if (configExist)
    {
        int Scale = GetPrivateProfileInt(TEXT("Resolution"), TEXT("RES"), 2, TEXT(CONFIG_INI));
        switch (Scale)
        {
        case 1:
            REC_WIDTH = 320;
            REC_HEIGHT = 240;
			break;
		case 2:
			REC_WIDTH = 640;
			REC_HEIGHT = 480;
			break;
        case 3:
            REC_WIDTH = 1280;
            REC_HEIGHT = 1024;
			break;
        default:
			cout << "Resolution not supported!" << endl;
            break;
        }

		DEPTH_FOV_H = GetPrivateProfileInt(TEXT("FOV"), TEXT("Depth_H"), 0x00, TEXT(CONFIG_INI)) / 100.0f;
		DEPTH_FOV_V = GetPrivateProfileInt(TEXT("FOV"), TEXT("Depth_V"), 0x00, TEXT(CONFIG_INI)) / 100.0f;
	}

    Mat     cvRGBImg(REC_HEIGHT, REC_WIDTH, CV_8UC3);
    Mat     cvBGRImg(REC_HEIGHT, REC_WIDTH, CV_8UC3);
    Mat     cvIRImg(REC_HEIGHT, REC_WIDTH, CV_16UC1);
    Mat     c24bitIR(REC_HEIGHT, REC_WIDTH, CV_8UC3);
    Mat     cvDepthImg(REC_HEIGHT, REC_WIDTH, CV_16UC1);
    Mat     registerMat;
    Mat     CaliDepth(REC_HEIGHT, REC_WIDTH, CV_16UC1);
    Mat     CaliDepthHistogram(REC_HEIGHT, REC_WIDTH, CV_8UC3);
    Mat     CaliColor(REC_HEIGHT, REC_WIDTH, CV_8UC3);
    Mat     regErrLabelImg(REC_HEIGHT, REC_WIDTH, CV_8UC3);

    bool rgbMode = true;
    IplImage IplColor, IplDepth, IplReg;
    char chrDepthColorAlpha[32] = {0};


    Vector<IplImage> imgsOne(1);
    Vector<IplImage> imgsTwo(2);
    CvFont font;
    
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 1, CV_AA);
    
    cvNamedWindow(_winTitle, 1);

	Key key;
	Config config;

	// Init
	config.overlayDisplay = true;
	config.alphaColor = 50;
	config.alphaLabel = 50;
	config.registrationType = 1;
	config.savePointCloud = false;
	config.useDistortCoef = true;

	cv::Rect growingROI = cv::Rect(160, 120, 320, 240); // TODO: select ROI with mouse
	cv::Mat regErrLabelMap;

	CRGBDCamera camera(REC_WIDTH, REC_HEIGHT);

	if (readDepthFromFile)
	{
		config.registrationType = 3;

		if (File::Exist(CAMERA_PARAMS_INI))
		{
			softwareRegistrator.LoadParamsFromIniFile(CAMERA_PARAMS_INI);
			softwareRegistrator.CalcConversionMatrix();

			sprintf(strRegisterType, "[D2C]");
			strRegisterInfo = "Software Registration(load params from file)";
		}
		else
		{
			cout << "Param file not found!" << endl;
		}
	}
	else
	{
		camera.InitializeDevice();
		int res = camera.OpenDevice(uri);
		if (res != 0)
		{
			printf("Press any key to continue . . .\n");
			getchar();
			return 0;
		}

		camera.InitializeRGBorIRStream(rgbMode, REC_HEIGHT, REC_WIDTH);
		camera.StartRGBorIRStream();
		camera.StartDepthStream();
	}

    while (true)
    {
		// Process key
		int newKey = waitKey(10);
		int keyRes = key.Handle(config, newKey);
		if (keyRes != 0)
		{
			break;
		}

		// Get new frame
		if (readDepthFromFile)
		{
			cvDepthImg = cv::imread("depth.png",CV_LOAD_IMAGE_UNCHANGED);
			cvRGBImg = cv::imread("rgb.bmp");
			rgbMode = true;
		}
		else
		{
			// Switch registration type
			SwitchRegistrationType(config.registrationType, camera);
			// Read new frame
			camera.getNextFrame(cvRGBImg, cvIRImg, cvDepthImg);
		}
      
		rgbMode == true ? cv::cvtColor(cvRGBImg, cvBGRImg, CV_RGB2BGR) : camera.IRConvertWORD2BYTE(cvIRImg, c24bitIR);
		
		// Process depth image
		switch (config.registrationType)
		{
		case 1:
			mDraw.GetDepthHistogram(cvDepthImg, CaliDepthHistogram);
			break;

		case 2:
			mDraw.GetDepthHistogram(cvDepthImg, CaliDepthHistogram);
			break;

		case 3:
		case 4:
			softwareRegistrator.MappingDepth2Color(cvDepthImg, CaliDepth, config.useDistortCoef);
			mDraw.GetDepthHistogram(CaliDepth, CaliDepthHistogram);
			break;

		case 5:
			mDraw.GetDepthHistogram(cvDepthImg, CaliDepthHistogram);

			softwareRegistrator.MappingDepth2Color(cvDepthImg, CaliDepth, config.useDistortCoef);
			softwareRegistrator.MappingColor2Depth(CaliDepth, cvBGRImg, cvBGRImg);
			break;

		default:
			break;
		}


		cv::cvtColor(regErrLabelMap, regErrLabelImg, CV_GRAY2BGR);

		double colorRatio = config.alphaColor / 100.0;
		cv::addWeighted(CaliDepthHistogram, 1 - colorRatio, cvBGRImg, colorRatio, 0.5, registerMat);
		double labelRatio = config.alphaLabel / 100.0;

		if (config.savePointCloud)
		{
			config.savePointCloud = false; // Save single frame each time

			if (config.registrationType != 3 && config.registrationType != 4 && config.registrationType != 5)
			{
				cout << "Please turn on software registration before saving point cloud!" << endl;
			}
			else
			{
				pointCloud.pSoftReg = &softwareRegistrator;
				pointCloud.update(CaliDepth, cvRGBImg, config.useDistortCoef);
				pointCloud.save("PointCloud.ply");
			}
		}

		if (config.overlayDisplay == true)
		{
			IplReg = registerMat;
			imgsOne[0] = IplReg;
		}
		else
		{
			IplReg = cvBGRImg;
			imgsTwo[0] = IplReg;
			imgsTwo[1] = CaliDepthHistogram;
		}

        // Draw info on windows

		cvPutText(&IplReg, strRegisterType, cvPoint(10, 380), &font, cvScalar(0, 0, 255, 255));
		cvPutText(&IplReg, strRegisterInfo, cvPoint(10, 410), &font, cvScalar(0, 0, 255, 255));

		if (config.registrationType == 3 || config.registrationType == 4)
		{
			if (config.useDistortCoef)
			{
				strUseDistortion = "Use distortion parameters true";
			}
			else
			{
				strUseDistortion = "Use distortion parameters false";
			}
		}
		cvPutText(&IplReg, strUseDistortion, cvPoint(10, 440), &font, cvScalar(0, 0, 255, 255));

		sprintf(chrDepthColorAlpha, "Alpha: color %d label %d", config.alphaColor, config.alphaLabel);
		cvPutText(&IplReg, chrDepthColorAlpha, cvPoint(10, 470), &font, cvScalar(0, 0, 255, 255));

        mDraw.ShowImagesSideBySide(_winTitle, config.overlayDisplay == true ? imgsOne : imgsTwo, "", 10, 450);
    }

	if (!readDepthFromFile)
	{
		camera.StopRGBorIRStream();
		camera.StopDepthStream();
		camera.CloseDevice();
	}

    return 0;
}
