#pragma once

#include "OpenNI.h"
#include "opencv/cv.h"

#define SAMPLE_READ_WAIT_TIMEOUT 100 //100ms

typedef struct OBCameraParams
{
    float l_intr_p[4];//[fx,fy,cx,cy]
    float r_intr_p[4];//[fx,fy,cx,cy]
    float r2l_r[9];//[r00,r01,r02;r10,r11,r12;r20,r21,r22]
    float r2l_t[3];//[t1,t2,t3]
	float l_k[5];//[k1,k2,p1,p2,k3]
	float r_k[5];
    int is_mirror;
}OBCameraParams;

class CRGBDCamera
{
public:
    CRGBDCamera(int width, int height);
    virtual ~CRGBDCamera();

    int InitializeDevice();
	int OpenDevice(const char* uri);
    int CloseDevice();
    int InitializeRGBorIRStream(bool rgbMode, int ir_h, int ir_w);
    int StartRGBorIRStream();
    int StartDepthStream();
    int StopRGBorIRStream();
    int StopDepthStream();
    int getNextFrame(cv::Mat& color, cv::Mat& ir);
    int getNextFrame(cv::Mat& color, cv::Mat& ir, cv::Mat& depth);
    int IRConvertWORD2BYTE(cv::Mat &src, cv::Mat &dst);

    bool toggleRegister(bool setRegister);

	int LoadCameraParams(OBCameraParams& params);
	void SetCameraParams(OBCameraParams& params);

private:

    int m_ir;
    int m_W;
    int m_H;

    openni::Device m_device;
    openni::VideoStream m_depth;
    openni::VideoStream m_rgb_ir;
    openni::VideoFrameRef m_rgb_ir_frame;
    openni::VideoFrameRef m_depth_frame;


    openni::VideoMode m_rgb_ir_vmod;
    openni::VideoMode m_depth_vmod;

};

