

#include "OpenNI.h"
#include "opencv/cv.h"
#include "XnUSB.h"

using namespace openni;
using namespace cv;

#define SAMPLE_READ_WAIT_TIMEOUT 100 //100ms



#pragma once
class CRGBDCamera
{
public:
    CRGBDCamera();
    virtual ~CRGBDCamera();

    int InitializeDevice();
    int OpenDevice();
    int CloseDevice();
    int InitializeRGBorIRStream(bool rgbMode, int ir_h, int ir_w);
    int StartRGBorIRStream();
    int StopRGBorIRStream();
    int getNextFrame(cv::Mat& color, cv::Mat& ir, cv::Mat& yuv);
    int IRConvertWORD2BYTE(cv::Mat &src, cv::Mat &dst);
    char* getSerialNumber();

    int EnumDevice(uint16_t &vid, uint16_t &pid);

private:
    int m_ir;
    char m_SerialNumber[13];

    uint16_t m_vid;
    uint16_t m_pid;

    const XnUSBConnectionString* m_astrDevicePaths;

    Device m_device;
    VideoStream m_depth;
    VideoStream m_rgb_ir;
    VideoFrameRef m_frame;
    VideoMode m_rgb_ir_vmod;
};

