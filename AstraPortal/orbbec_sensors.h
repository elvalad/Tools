#ifndef ORBBEC_GLOBAL_H
#define ORBBEC_GLOBAL_H

#include <QtGlobal>
#include <OpenNI.h>
#include <iostream>
#include <vector>
#include <list>

#include "orbbec_common.h"

using namespace openni;
using namespace std;

#define MAX_DEPTH 10000

class Sensors
{
public:
    Sensors();
    ~Sensors();

    int initCurSensor();
    void deinitCurSensor();

    void getSensorList(Array<DeviceInfo>& vList);
    void getCurSensorInfo(SensorInfo_S& sensorInfo);
    void toggleImageRegistration();
    void calculateHistogram(float* pHistogram, int histogramSize, const openni::VideoFrameRef& frame);

    int startCurColor();
    void stopCurColor();
    VideoMode getCurColorVideoMode();
    Resolution_S getCurColorReso();
    uchar* getCurColorData(bool bOverlay);
    void toggleColorMirror(int state);
    void toggleAutoWhiteBalance(int state);
    void toggleAutoExposure(int state);

    int startCurDepth();
    void stopCurDepth();
    VideoMode getCurDepthVideoMode();
    Resolution_S getCurDepthReso();
    uchar* getCurDepthData();
    const DepthPixel* getCurDepthValue();
    void toggleDepthMirror(int state);

    void generatePointCloud(vector<ColorPoint3D_S>& vPointCloud, bool bUseOwnParams, bool bUseExternalParams, int cropX, int cropY);

    int initChangedSensor(const char* uri);

    void parseParamsFile();
    void convertProjectiveToWorld(int u, int v, int z, float& px, float& py, float& pz);
    void convertWorldToProjective(float x, float y, float z, float& u, float& v);
    void transformPointToPointByOwn(float dst[3], const float src[3]);
    void transformPointToPointByExternal(float dst[3], const float src[3]);

private:
    Array<DeviceInfo> m_sensorList;
    Device m_curSensor;
    SensorParams_S m_curSensorParams;
    bool bParamsFileExist;

    VideoStream m_curColorStream;
    VideoFrameRef m_curColorFrame;
    VideoMode m_curColorVideoMode;
    uchar* m_pColorRGB888;
    Resolution_S m_curColorReso;

    VideoStream m_curDepthStream;
    VideoFrameRef m_curDepthFrame;
    VideoMode m_curDepthVideoMode;
    uchar* m_pDepthARGB;
    Resolution_S m_curDepthReso;

    RGB888Pixel* m_pTexMap;
    float m_pDepthHist[MAX_DEPTH];
    int m_width;
    int m_height;
    int m_nTexMapX;
    int m_nTexMapY;
};

#endif // ORBBEC_GLOBAL_H
