

#include "OpenNI.h"
#include <iostream>
#include <string>
#include <math.h>

// OpenCV
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>

#include "RGB-D-calibration-sample-code.h"
#include "CIniFile.h"

#define CAMERA_PARAMS_INI   "./camera_params.ini"
#define CONFIG_INI          "./config.ini"

using namespace std;
using namespace cv;
using namespace openni;


void cvShowManyImages(char* title, int nArgs, std::vector<IplImage> imgs, int width, int height)
{
    IplImage* img = nullptr;
    IplImage* DispImage = nullptr;

    int Ysize = height;
    int Xsize = width;
    float scale;

    int row_num = 2, col_num = 1;
    if (nArgs == 1){
        row_num = 1, col_num = 1;
    }
    else if (nArgs == 2){
        row_num = 2, col_num = 1;
    }

    DispImage = cvCreateImage(cvSize(Xsize * row_num, Ysize * col_num), 8, 3);

    int m = 0, n = 0;
    for (int i = 0; i < nArgs; i++, m += (Xsize)) {
        img = &imgs[i];
        if (img == 0) {
            printf("Invalid arguments");
            cvReleaseImage(&DispImage);
            return;
        }

        cvSetImageROI(DispImage, cvRect(m, n, Xsize, Ysize));
        cvResize(img, DispImage);
        cvResetImageROI(DispImage);
    }

    // Create a new window, and show the Single Big Image
    cvNamedWindow(title, 1);
    cvShowImage(title, DispImage);

    cvReleaseImage(&DispImage);
}

void CheckOpenNIError(Status result, string status)
{
    if (result != STATUS_OK)
        cerr << status << " Error: " << OpenNI::getExtendedError() << endl;
}

void GetDepthHistogram(cv::Mat &src, cv::Mat &dst)
{
    float depthHistogram[65536];
    int numberOfPoints = 0;
    cv::Mat depthHist(src.rows, src.cols, CV_8UC3);
    memset(depthHistogram, 0, sizeof(depthHistogram));
    for (int y = 0; y < src.rows; ++y)
    {
        ushort* depthCell = (ushort*)src.ptr<uchar>(y);
        for (int x = 0; x < src.cols; ++x)
        {
            if (*depthCell != 0)
            {
                depthHistogram[*depthCell]++;
                numberOfPoints++;
            }
            depthCell++;
        }
    }

    for (int nIndex = 1; nIndex < sizeof(depthHistogram) / sizeof(int); nIndex++)
    {
        depthHistogram[nIndex] += depthHistogram[nIndex - 1];
    }
    for (int nIndex = 1; nIndex < sizeof(depthHistogram) / sizeof(int); nIndex++)
    {
        depthHistogram[nIndex] = (numberOfPoints - depthHistogram[nIndex]) / numberOfPoints;
    }
    for (int y = 0; y < src.rows; ++y)
    {
        ushort* depthCell = (ushort*)src.ptr<uchar>(y);
        uchar * showcell = (uchar *)depthHist.ptr<uchar>(y);
        for (int x = 0; x < src.cols; ++x)
        {
            char depthValue = depthHistogram[*depthCell] * 255;
            *showcell++ = 0;
            *showcell++ = depthValue;
            *showcell++ = depthValue;
            //*showcell++ = depthValue;

            depthCell++;
        }
    }
    dst = depthHist;
}

bool checkFile(const char * file)
{
    bool r = false;

    if (access(file, 0) != -1)
    {
        cout << "-- find " << file << endl << endl;
        r = true;
    }
    else
    {
        cout << "-- Didn't find " << file << endl << endl;
    }

    return r;
}

void TOsxga(cv::Mat& color)
{
    int rgb_W = color.rows;
    int rgb_h = color.cols;

    cv::Mat colorSXGA(1024, 1280, CV_8UC3);
    uint8_t blank = (1024 - 720) / 2 - 1;
    for (int i = 0; i < color.rows; ++i)
    {
        for (int j = 0; j < color.cols; ++j)
        {
            Vec3b p = color.at<Vec3b>(i, j);
            colorSXGA.at<Vec3b>(i + blank - 1, j) = p;
        }
    }
    color = colorSXGA;
}

int main(int argc, char** argv)
{
    Status result = STATUS_OK;

    int DISPLAY_WIDTH = 640;
    int DISPLAY_HEIGHT = 480;

    if (checkFile(CONFIG_INI))
    {

        if( !OpenIniFile(CONFIG_INI) ) return false;

        int Scale = ReadInt("Resolution","RES",0);

        if (Scale == 2)
        {
            DISPLAY_WIDTH = 640;
            DISPLAY_HEIGHT = 480;
        }
        if (Scale == 3)
        {
            DISPLAY_WIDTH = 1280;
            DISPLAY_HEIGHT = 1024;
        }
    }

    VideoCapture capture;
    if (!capture.open(0))
    {
        capture.open(1);
    }

    if (capture.isOpened())
    {
        if (DISPLAY_WIDTH == 1280 && DISPLAY_HEIGHT == 1024)
        {
            capture.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
            capture.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
        }
        {
            capture.set(CV_CAP_PROP_FRAME_WIDTH, DISPLAY_WIDTH);
            capture.set(CV_CAP_PROP_FRAME_HEIGHT, DISPLAY_HEIGHT);
        }
    }

    VideoFrameRef oniDepthFrameRef;

    IplImage    IplDepth;
    Mat         ColorImg;
    Mat         depthImg(DISPLAY_HEIGHT, DISPLAY_WIDTH, CV_16UC1);
    Mat         flipDepthImg(DISPLAY_HEIGHT, DISPLAY_WIDTH, CV_16UC1);
    Mat         CaliDepth(DISPLAY_HEIGHT, DISPLAY_WIDTH, CV_16UC1);
    Mat         CaliDepthHistogram(DISPLAY_HEIGHT, DISPLAY_WIDTH, CV_16UC1);
    Mat         overMat;
    char        key = 0;
    CvFont      font;
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 1, CV_AA);
    bool        IsOver = true;
    bool        IsRegister = false;
    char        temp_alpha[32];
    uint16_t    alphaC = 5;
    uint16_t    alphaD = 5;
    float       mat[16] = { 0 };
    bool        isCameraParamsExist = true;
    //Mat       cv8bitDepth(DISPLAY_HEIGHT, DISPLAY_WIDTH, CV_8UC1);
    //Mat       cv24bitDepth(DISPLAY_HEIGHT, DISPLAY_WIDTH, CV_8UC3);

    result = OpenNI::initialize();
    CheckOpenNIError(result, "initialize context");

    std::cout << " *********** Astra Pro Viewer v1.0*********" << endl << endl;

    std::cout << " O - RGBD Over display" << endl;
    std::cout << " R - toggle register" << endl;
    std::cout << " C - increase the Color proportion (after 'O')" << endl;
    std::cout << " D - increase the Depth proportion (after 'O')" << endl;
    std::cout << " Esc - Exit" << endl << endl;

    Device device;
    result = device.open(openni::ANY_DEVICE);

    VideoStream oniDepthStream;
    result = oniDepthStream.create(device, openni::SENSOR_DEPTH);
    CheckOpenNIError(result, "depth stream creat false");

    VideoMode modeDepth;
    modeDepth.setResolution(DISPLAY_WIDTH, DISPLAY_HEIGHT);
    modeDepth.setFps(30);
    modeDepth.setPixelFormat(PIXEL_FORMAT_DEPTH_1_MM);
    oniDepthStream.setVideoMode(modeDepth);
    //oniDepthStream.setMirroringEnabled(true);
    result = oniDepthStream.start();


    isCameraParamsExist = checkFile(CAMERA_PARAMS_INI);
    if (isCameraParamsExist)
    {
        CameraParameter camera;
        camera.loadCameraParams((char*)CAMERA_PARAMS_INI);

        CalcALLMat(mat, camera);
    }

    cvNamedWindow("Astra Pro Viewer", 1);

    while (true)
    {
        capture >> ColorImg;
        if (ColorImg.empty()){
            break;
        }

        if (DISPLAY_WIDTH == 1280 && DISPLAY_HEIGHT == 1024)
        {
            TOsxga(ColorImg);
        }

        result = oniDepthStream.readFrame(&oniDepthFrameRef);
        CheckOpenNIError(result, "Read failed!\n%s\n");

        uint16_t *pPixel;
        for (unsigned int y = 0; y < DISPLAY_HEIGHT; y++)
        {
            pPixel = ((uint16_t*)((char*)oniDepthFrameRef.getData() + ((int)(y)* oniDepthFrameRef.getStrideInBytes())));
            uint16_t* data = (uint16_t*)depthImg.ptr<uchar>(y);
            for (unsigned int x = 0; x < DISPLAY_WIDTH; x++)
            {
                *data++ = (*pPixel);
                pPixel++;
            }
        }

        //depthImg.convertTo(cv8bitDepth, CV_8UC1, 255.0 / (8000));
        //cvtColor(cv8bitDepth, cv24bitDepth, CV_GRAY2RGB);
        //cv::imshow("Depth", cv24bitDepth);

        cv::flip(depthImg, flipDepthImg, 1);
        if (isCameraParamsExist)
        {
            MappingDepth2Color(flipDepthImg, CaliDepth, mat);
            GetDepthHistogram(CaliDepth, CaliDepthHistogram);
        }
        else
        {
            GetDepthHistogram(flipDepthImg, CaliDepthHistogram);
        }
        //imshow("CaliDepthHistogram", CaliDepthHistogram);

        cv::addWeighted(CaliDepthHistogram, (double)(alphaD / 10.0), ColorImg, (double)(alphaC / 10.0), 0.5, overMat);
        //imshow("overMat", overMat);

        if (IsOver)
        {
            IplDepth = overMat;
            sprintf(temp_alpha, "C-D: %d-%d", alphaC, alphaD);

            if (IsRegister)
            {
                cvPutText(&IplDepth, "hard Registered", cvPoint(10, DISPLAY_WIDTH * 1 / 2), &font, cvScalar(0, 0, 255, 255));
            }
            if (isCameraParamsExist)
            {
                cvPutText(&IplDepth, "soft Registered", cvPoint(10, DISPLAY_WIDTH * 1 / 2), &font, cvScalar(0, 0, 255, 255));
            }

            cvPutText(&IplDepth, temp_alpha, cvPoint(10, 30), &font, cvScalar(0, 0, 255, 255));

            std::vector<IplImage> imgs(1);
            imgs[0] = overMat;
            cvShowManyImages((char*)"Astra Pro Viewer", 1, imgs, DISPLAY_WIDTH, DISPLAY_HEIGHT);
            oniDepthStream.setMirroringEnabled(true);
        }
        else{
            IplDepth = CaliDepthHistogram;
            if (IsRegister)
            {
                cvPutText(&IplDepth, "hard Registered", cvPoint(10, DISPLAY_WIDTH * 1 / 2), &font, cvScalar(0, 0, 255, 255));
            }
            if (isCameraParamsExist)
            {
                cvPutText(&IplDepth, "soft Registered", cvPoint(10, DISPLAY_WIDTH * 1 / 2), &font, cvScalar(0, 0, 255, 255));
            }

            std::vector<IplImage> imgs(2);
            imgs[0] = ColorImg;
            imgs[1] = CaliDepthHistogram;
            cvShowManyImages((char*)"Astra Pro Viewer", 2, imgs, DISPLAY_WIDTH, DISPLAY_HEIGHT);
            oniDepthStream.setMirroringEnabled(true);
        }

        if (key == 'o' || key == 'O'){
            IsOver = !IsOver;
        }
        else if (key == 'r' || key == 'R')
        {
            if (!isCameraParamsExist)
            {
                if (DISPLAY_WIDTH != 1280 && DISPLAY_HEIGHT != 1024)
                {
                    if (!IsRegister)
                    {
                        if (device.isImageRegistrationModeSupported(IMAGE_REGISTRATION_DEPTH_TO_COLOR)){
                            device.setImageRegistrationMode(IMAGE_REGISTRATION_DEPTH_TO_COLOR);
                            IsRegister = true;
                        }
                    }
                    else{
                        device.setImageRegistrationMode(IMAGE_REGISTRATION_OFF);
                        IsRegister = false;
                    }
                }
            }
        }
        else if (key == 'D' || key == 'd')
        {
            if (alphaD >= 0 && alphaC >= 1)
                alphaD += 1, alphaC -= 1;
        }
        else if (key == 'c' || key == 'C')
        {
            if (alphaD >= 1 && alphaC >= 0)
                alphaD -= 1, alphaC += 1;
        }
        else if (key == 27)
        {
            break;
        }
        key = waitKey(1);
    }

    oniDepthStream.destroy();
    device.close();
    OpenNI::shutdown();
    return 0;
}
