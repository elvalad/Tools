

#include "OpenNI.h" 
#include <iostream>
#include <string>
#include <math.h>
#include <io.h>

#include "RGBDCamera.h"

// OpenCV Header
#include "opencv/highgui.h"
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"


using namespace std;
using namespace cv;
using namespace openni;

#define XN_FILE_MAX_PATH            260
#define CONFIG_INI                  "./AstraCaptureConfig.ini"

int capturedFrameUniqueID;

bool checkFile(char * file)
{
    bool r = false;

    if (_access(file, 0) != -1)
    {
        r = true;
    }

    return r;
}

int findUniqueDirName(char* dirName)
{
    int num = 0;

    char csDirName[XN_FILE_MAX_PATH];

    for (;;)
    {
        sprintf(csDirName, "%s_%d", dirName, num);

        bool bExist = checkFile(csDirName);

        if (!bExist)
            break;

        ++num;
    }
    return num;
}

int findUniqueFileName(char* dir)
{
    int num = capturedFrameUniqueID;

    char csColorFileName[XN_FILE_MAX_PATH];
    char csIRFileName[XN_FILE_MAX_PATH];

    for (;;)
    {
        sprintf(csColorFileName, "%s/right%d.bmp", dir, num);
        sprintf(csIRFileName, "%s/left%d.bmp", dir, num);

        bool bColorExist = checkFile(csColorFileName);
        bool bIRExist = checkFile(csIRFileName);

        if (!bColorExist && !bIRExist)
            break;

        ++num;
    }
    return num;
}

int main()
{
    Status result = STATUS_OK;

    int DISPLAY_WIDTH = 640;
    int DISPLAY_HEIGHT = 480;
    bool isSaveYUV = false;

    if (checkFile(CONFIG_INI))
    {
        int Scale = GetPrivateProfileInt(TEXT("Resolution"), TEXT("RES"), 2, TEXT(CONFIG_INI));
        int yue = GetPrivateProfileInt(TEXT("YUV"), TEXT("SaveYUV"), 0, TEXT(CONFIG_INI));
        if (yue == 1)
        {
            isSaveYUV = true;
        }
        else
        {
            isSaveYUV = false;
        }

        if (Scale == 2)
        {
            DISPLAY_WIDTH = 640;
            DISPLAY_HEIGHT = 480;
        }
        else if (Scale == 3)
        {
            DISPLAY_WIDTH = 1280;
            DISPLAY_HEIGHT = 1024;
        }
    }

    Mat     irImg(DISPLAY_HEIGHT, DISPLAY_WIDTH, CV_16UC1);
    Mat     c24bitIR(DISPLAY_HEIGHT, DISPLAY_WIDTH, CV_8UC3);
    Mat     cvRGBImg(DISPLAY_HEIGHT, DISPLAY_WIDTH, CV_8UC3);
    Mat     cvYUVImg(DISPLAY_HEIGHT, DISPLAY_WIDTH, CV_8UC3);
    Mat     cvBGRImg(DISPLAY_HEIGHT, DISPLAY_WIDTH, CV_8UC3);

    char    ImagesName[1024];
    char    key = 0;
    bool    rgbMode = false;
    bool    MultCaptureMode = false;
    int     MultiCaptureIndex;
    char    MultiCaptureSeq[2] = { 'I', 'I' };
    char    csDirName[XN_FILE_MAX_PATH];
    char    cmdName[XN_FILE_MAX_PATH];
    uint16_t vid;
    uint16_t pid;
    
    CRGBDCamera camera;
    camera.InitializeDevice();
    camera.OpenDevice();
    camera.InitializeRGBorIRStream(rgbMode, DISPLAY_HEIGHT, DISPLAY_WIDTH);
    camera.StartRGBorIRStream();

    camera.EnumDevice(vid, pid);
    if (pid == 0x0403)
    {
        printf("This is UVC camera, OpenNI2 does not support UVC camera\n");
        printf("Press any key to continue . . .\n");
        getchar();
        return 0;
    }

    char(*SerialNumber) = camera.getSerialNumber();
    
    std::cout << "******************** Astra Capture v1.2 ********************" << endl << endl;
    cout << "S -- Save pictures" << endl;
    cout << "I -- Switch the view" << endl << endl;

    std::cout << "Device SN: " << SerialNumber << endl;

    int dirNum = findUniqueDirName(SerialNumber);
    sprintf(csDirName, "%s_%d", SerialNumber, dirNum);
    std::cout << "Images will be saved in folder:" << csDirName << endl << endl;
    
    sprintf(cmdName, "md %s", csDirName);
    system(cmdName);

    while (true)
    {
        camera.getNextFrame(cvRGBImg, irImg, cvYUVImg);

        if (rgbMode)
        {
            cv::cvtColor(cvRGBImg, cvBGRImg, CV_RGB2BGR);
            cv::imshow("Preview", cvBGRImg);
            if (isSaveYUV)
            {
                cv::imshow("Preview_yuv", cvYUVImg);
            }

        }
        else
        {
            camera.IRConvertWORD2BYTE(irImg, c24bitIR);
            cv::imshow("Preview", c24bitIR);
        }

        if (MultCaptureMode)
        {
            key = 'I';
            MultiCaptureIndex++;
            if (MultiCaptureIndex == 2)
            {
                MultCaptureMode = false;
            }

            if (rgbMode)
            {
                IplImage* img = new IplImage(cvBGRImg);
				sprintf(ImagesName, "%s/right%d.bmp", csDirName, capturedFrameUniqueID);
				cvSaveImage(ImagesName, img);
                delete img;

                if (isSaveYUV)
                {
                    IplImage* imgyuv = new IplImage(cvYUVImg);
					sprintf(ImagesName, "%s/yuv%d.bmp", csDirName, capturedFrameUniqueID);
					cvSaveImage(ImagesName, imgyuv);
                    delete imgyuv;
                }

                cout << "saved rgb image " << ImagesName << endl;
            }
            else
            {
                sprintf(ImagesName, "%s/left%d.bmp", csDirName, capturedFrameUniqueID);

                IplImage* img = new IplImage(c24bitIR);
                cvSaveImage(ImagesName, img);
                delete img;
                cout << "saved ir image " << ImagesName << endl;
            }
        }
        else
        {
            key = cv::waitKey(10);
        }

        if (key == 'I' || key == 'i')
        {
            rgbMode = !rgbMode;
            camera.InitializeRGBorIRStream(rgbMode, DISPLAY_HEIGHT, DISPLAY_WIDTH);
            camera.StartRGBorIRStream();

            waitKey(1000); // Wait until stream is stable
        }
        else if (key == 'S' || key == 's')
        {
            capturedFrameUniqueID = findUniqueFileName(csDirName);
            MultCaptureMode = true;
            MultiCaptureIndex = 0;
        }
        else if (key == 27)
        {
            break;
        }
    }

    camera.StopRGBorIRStream();
    camera.CloseDevice();

    cout << "stopped" << endl;

    return 0;
}
