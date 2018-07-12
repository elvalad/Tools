#include <OpenNI.h>
#include <iostream>
#include <stdint.h>
#include <fstream>
#include <omp.h>
#include <vector>
#include <QFile>
#include <QFileInfo>

#include "orbbec_sensors.h"
#include "orbbec_util.h"

using namespace openni;
using namespace std;

Sensors::Sensors()
{
    Status ret = STATUS_OK;
    ret = OpenNI::initialize();
    cout << "OpenNI initialization : " << OpenNI::getExtendedError() << endl;
}

Sensors::~Sensors()
{
    if (m_curColorStream.isValid())
    {
        cout << "Destroy current color stream." << endl;
        m_curColorStream.destroy();
    }

    if (m_curDepthStream.isValid())
    {
        cout << "Destroy current depth stream." << endl;
        m_curDepthStream.destroy();
    }

    if (m_curSensor.isValid())
    {
        cout << "Close current sensor." << endl;
        m_curSensor.close();
    }

    OpenNI::shutdown();
    cout << "OpenNI shutdown." << endl;
}

void Sensors::parseParamsFile()
{
    QString paramFileName = "./camera_params_";
    char serialNum[12];
    int dataSize = sizeof(serialNum);
    m_curSensor.getProperty((int)ONI_DEVICE_PROPERTY_SERIAL_NUMBER, &serialNum, &dataSize);
    paramFileName.append(serialNum).append(".ini");
    QFileInfo fileInfo(paramFileName);
    if (fileInfo.isFile())
    {
        cout << "Param file name : " << paramFileName.toStdString() << endl;
        bParamsFileExist = true;
        char tmpbuf[128];
        float tmp;
        std::ifstream inf;
        inf.open(paramFileName.toStdString(), std::ios::in);
        inf.getline(tmpbuf, sizeof(tmpbuf));
        inf >> m_curSensorParams.l_intr_p[0] >> tmp >> m_curSensorParams.l_intr_p[2] >>
               tmp >> m_curSensorParams.l_intr_p[1] >> m_curSensorParams.l_intr_p[3] >>
               tmp >> tmp >> tmp;

        inf.getline(tmpbuf, sizeof(tmpbuf));
        inf.getline(tmpbuf, sizeof(tmpbuf));
        inf >> m_curSensorParams.r_intr_p[0] >> tmp >> m_curSensorParams.r_intr_p[2] >>
               tmp >> m_curSensorParams.r_intr_p[1] >> m_curSensorParams.r_intr_p[3] >>
               tmp >> tmp >> tmp;

        inf.getline(tmpbuf, sizeof(tmpbuf));
        inf.getline(tmpbuf, sizeof(tmpbuf));
        inf >> tmp; m_curSensorParams.o_r2l_r[0] = tmp;
        inf >> tmp; m_curSensorParams.o_r2l_r[1] = tmp;
        inf >> tmp; m_curSensorParams.o_r2l_r[2] = tmp;
        inf >> tmp; m_curSensorParams.o_r2l_r[3] = tmp;
        inf >> tmp; m_curSensorParams.o_r2l_r[4] = tmp;
        inf >> tmp; m_curSensorParams.o_r2l_r[5] = tmp;
        inf >> tmp; m_curSensorParams.o_r2l_r[6] = tmp;
        inf >> tmp; m_curSensorParams.o_r2l_r[7] = tmp;
        inf >> tmp; m_curSensorParams.o_r2l_r[8] = tmp;

        inf.getline(tmpbuf, sizeof(tmpbuf));
        inf.getline(tmpbuf, sizeof(tmpbuf));
        inf >> tmp; m_curSensorParams.o_r2l_t[0] = tmp;
        inf >> tmp; m_curSensorParams.o_r2l_t[1] = tmp;
        inf >> tmp; m_curSensorParams.o_r2l_t[2] = tmp;

        inf.getline(tmpbuf, sizeof(tmpbuf));
        inf.getline(tmpbuf, sizeof(tmpbuf));
        inf >> tmp; m_curSensorParams.e_r2l_r[0] = tmp;
        inf >> tmp; m_curSensorParams.e_r2l_r[1] = tmp;
        inf >> tmp; m_curSensorParams.e_r2l_r[2] = tmp;
        inf >> tmp; m_curSensorParams.e_r2l_r[3] = tmp;
        inf >> tmp; m_curSensorParams.e_r2l_r[4] = tmp;
        inf >> tmp; m_curSensorParams.e_r2l_r[5] = tmp;
        inf >> tmp; m_curSensorParams.e_r2l_r[6] = tmp;
        inf >> tmp; m_curSensorParams.e_r2l_r[7] = tmp;
        inf >> tmp; m_curSensorParams.e_r2l_r[8] = tmp;

        inf.getline(tmpbuf, sizeof(tmpbuf));
        inf.getline(tmpbuf, sizeof(tmpbuf));
        inf >> tmp; m_curSensorParams.e_r2l_t[0] = tmp;
        inf >> tmp; m_curSensorParams.e_r2l_t[1] = tmp;
        inf >> tmp; m_curSensorParams.e_r2l_t[2] = tmp;

        inf.close();
#if 0
        cout << "[IR Camera Intrinsic]" << endl;
        cout << m_curSensorParams.l_intr_p[0] << " 0 " << m_curSensorParams.l_intr_p[2] << endl;
        cout << "0 " << m_curSensorParams.l_intr_p[1] << " " << m_curSensorParams.l_intr_p[3] << endl;
        cout << "0" << " 0 " << "1" << endl;
        cout << "[RGB Camera Intrinsic]" << endl;
        cout << m_curSensorParams.r_intr_p[0] << " 0 " << m_curSensorParams.r_intr_p[2] << endl;
        cout << "0 " << m_curSensorParams.r_intr_p[1] << " " << m_curSensorParams.r_intr_p[3] << endl;
        cout << "0" << " 0 " << "1" << endl;
        cout << "[IR to RGB Camera Rotation Matrix]" << endl;
        cout << m_curSensorParams.o_r2l_r[0] << " " << m_curSensorParams.o_r2l_r[1] << " " << m_curSensorParams.o_r2l_r[2] << endl;
        cout << m_curSensorParams.o_r2l_r[3] << " " << m_curSensorParams.o_r2l_r[4] << " " << m_curSensorParams.o_r2l_r[5] << endl;
        cout << m_curSensorParams.o_r2l_r[6] << " " << m_curSensorParams.o_r2l_r[7] << " " << m_curSensorParams.o_r2l_r[8] << endl;
        cout << "[IR to RGB Camera Translattion Vector]" << endl;
        cout << m_curSensorParams.o_r2l_t[0] << " " << m_curSensorParams.o_r2l_t[1] << " " << m_curSensorParams.o_r2l_t[2] << endl;
        cout << "[IR to External Camera Rotation Matrix]" << endl;
        cout << m_curSensorParams.e_r2l_r[0] << " " << m_curSensorParams.e_r2l_r[1] << " " << m_curSensorParams.e_r2l_r[2] << endl;
        cout << m_curSensorParams.e_r2l_r[3] << " " << m_curSensorParams.e_r2l_r[4] << " " << m_curSensorParams.e_r2l_r[5] << endl;
        cout << m_curSensorParams.e_r2l_r[6] << " " << m_curSensorParams.e_r2l_r[7] << " " << m_curSensorParams.e_r2l_r[8] << endl;
        cout << "[IR to External Camera Translattion Vector]" << endl;
        cout << m_curSensorParams.e_r2l_t[0] << " " << m_curSensorParams.e_r2l_t[1] << " " << m_curSensorParams.e_r2l_t[2] << endl;
#endif
    }
    else
    {
        cout << "Param file not exist" << endl;
        bParamsFileExist = false;
    }
}

int Sensors::initCurSensor()
{
    const char* deviceURI = ANY_DEVICE;
    Status ret = STATUS_OK;
    ret = m_curSensor.open(deviceURI);
    if (STATUS_OK != ret)
    {
        cout << "Device open failed : " << OpenNI::getExtendedError() << endl;
        return -1;
    }

    parseParamsFile();

    return 0;
}

void Sensors::deinitCurSensor()
{
    if (m_curSensor.isValid())
    {
        m_curSensor.close();
    }
}

void Sensors::getSensorList(Array<DeviceInfo>& vList)
{
    OpenNI::enumerateDevices(&vList);
}

void Sensors::getCurSensorInfo(SensorInfo_S& sensorInfo)
{
    sensorInfo.deviceInfo = m_curSensor.getDeviceInfo();
    if (m_curColorStream.isValid())
    {
        sensorInfo.colorVideoMode = m_curColorStream.getVideoMode();
    }

    if (m_curDepthFrame.isValid())
    {
        sensorInfo.depthVideoMode = m_curDepthStream.getVideoMode();
    }

    int dataSize = sizeof(sensorInfo.serialNum);
    m_curSensor.getProperty((int)ONI_DEVICE_PROPERTY_SERIAL_NUMBER, &sensorInfo.serialNum, &dataSize);
}

void Sensors::toggleImageRegistration()
{
    ImageRegistrationMode mode = m_curSensor.getImageRegistrationMode();
    ImageRegistrationMode newMode = IMAGE_REGISTRATION_OFF;
    if (mode == IMAGE_REGISTRATION_OFF)
    {
        newMode = IMAGE_REGISTRATION_DEPTH_TO_COLOR;
    }

    if (m_curSensor.isImageRegistrationModeSupported(newMode))
    {
        m_curSensor.setImageRegistrationMode(newMode);
    }
    else
    {
        cout << "Couldn't change image registration to unsupported mode!" << endl;
    }
}

void Sensors::calculateHistogram(float* pHistogram, int histogramSize, const VideoFrameRef& frame)
{
    const DepthPixel* pDepth = (const DepthPixel*)frame.getData();
    // Calculate the accumulative histogram (the yellow display...)
    memset(pHistogram, 0, histogramSize*sizeof(float));
    int restOfRow = frame.getStrideInBytes() / sizeof(DepthPixel) - frame.getWidth();
    int height = frame.getHeight();
    int width = frame.getWidth();

    unsigned int nNumberOfPoints = 0;
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x, ++pDepth)
        {
            if (*pDepth != 0)
            {
                pHistogram[*pDepth]++;
                nNumberOfPoints++;
            }
        }
        pDepth += restOfRow;
    }
    for (int nIndex=1; nIndex<histogramSize; nIndex++)
    {
        pHistogram[nIndex] += pHistogram[nIndex-1];
    }
    if (nNumberOfPoints)
    {
        for (int nIndex=1; nIndex<histogramSize; nIndex++)
        {
            pHistogram[nIndex] = (256 * (1.0f - (pHistogram[nIndex] / nNumberOfPoints)));
        }
    }
}

int Sensors::startCurColor()
{
    Status ret = STATUS_OK;
    ret = m_curColorStream.create(m_curSensor, SENSOR_COLOR);
    if (STATUS_OK != ret)
    {
        cout << "Couldn't find color stream:" << OpenNI::getExtendedError() << endl;
        return -1;
    }
    else
    {
        ret = m_curColorStream.start();
        if (STATUS_OK != ret)
        {
            cout << "Couldn't start color stream:" << OpenNI::getExtendedError() << endl;
            m_curColorStream.destroy();
            return -2;
        }
    }

    if (!m_curColorStream.isValid())
    {
        cout << "No valid color streams." << OpenNI::getExtendedError() << endl;
        m_curColorStream.destroy();
        return -3;
    }

    m_curColorVideoMode = m_curColorStream.getVideoMode();
    m_curColorReso.w = m_curColorVideoMode.getResolutionX();
    m_curColorReso.h = m_curColorVideoMode.getResolutionY();
    m_pColorRGB888 = new uchar[3 * m_curColorReso.w * m_curColorReso.w];

    return 0;
}

void Sensors::stopCurColor()
{
    if (m_curColorStream.isValid())
    {
        delete[] m_pColorRGB888;
        m_curColorStream.destroy();
    }
}

VideoMode Sensors::getCurColorVideoMode()
{
    return m_curColorVideoMode;
}

Resolution_S Sensors::getCurColorReso()
{
    return m_curColorReso;
}

uchar* Sensors::getCurColorData(bool bOverlay)
{
    m_curColorStream.readFrame(&m_curColorFrame);
    if (bOverlay)
    {
        if (m_curColorReso.w != m_curDepthReso.w ||
                m_curColorReso.h != m_curDepthReso.h)
        {
            cout << "Expect color and depth to be in same resolution." << endl;
            return (uchar*)m_curColorFrame.getData();
        }

        calculateHistogram(m_pDepthHist, MAX_DEPTH, m_curDepthFrame);

        memset(m_pTexMap, 0, m_nTexMapX * m_nTexMapY * sizeof(RGB888Pixel));
        const RGB888Pixel* pImageRow = (const RGB888Pixel*)m_curColorFrame.getData();
        RGB888Pixel* pColorTexRow = m_pTexMap + m_curColorFrame.getCropOriginY() * m_nTexMapX;
        int colorRowSize = m_curColorFrame.getStrideInBytes() / sizeof(RGB888Pixel);

        for (int y = 0; y < m_curColorFrame.getHeight(); ++y)
        {
            const RGB888Pixel* pImage = pImageRow;
            RGB888Pixel* pTex = pColorTexRow + m_curColorFrame.getCropOriginX();

            for (int x = 0; x < m_curColorFrame.getWidth(); ++x, ++pImage, ++pTex)
            {
                *pTex = *pImage;
            }

            pImageRow += colorRowSize;
            pColorTexRow += m_nTexMapX;
        }

        const DepthPixel* pDepthRow = (const DepthPixel*)m_curDepthFrame.getData();
        RGB888Pixel* pDepthTexRow = m_pTexMap + m_curDepthFrame.getCropOriginY() * m_nTexMapX;
        int depthRowSize = m_curDepthFrame.getStrideInBytes() / sizeof(DepthPixel);

        for (int y = 0; y < m_curDepthFrame.getHeight(); ++y)
        {
            const DepthPixel* pDepth = pDepthRow;
            RGB888Pixel* pTex = pDepthTexRow + m_curDepthFrame.getCropOriginX();

            for (int x = 0; x < m_curDepthFrame.getWidth(); ++x, ++pDepth, ++pTex)
            {
                if (*pDepth != 0)
                {
                    int nHistValue = m_pDepthHist[*pDepth];
                    pTex->r = nHistValue;
                    pTex->g = nHistValue;
                    pTex->b = 0;
                }
            }

            pDepthRow += depthRowSize;
            pDepthTexRow += m_nTexMapX;
        }

        return (uchar*)m_pTexMap;
    }
    else
    {
        return (uchar*)m_curColorFrame.getData();
    }
}

void Sensors::toggleColorMirror(int state)
{
    m_curColorStream.setMirroringEnabled(!m_curColorStream.getMirroringEnabled());
}

void Sensors::toggleAutoWhiteBalance(int state)
{
    m_curColorStream.getCameraSettings()->setAutoWhiteBalanceEnabled(!m_curColorStream.getCameraSettings()->getAutoWhiteBalanceEnabled());
}

void Sensors::toggleAutoExposure(int state)
{
    m_curColorStream.getCameraSettings()->setAutoExposureEnabled(!m_curColorStream.getCameraSettings()->getAutoExposureEnabled());
}

int Sensors::startCurDepth()
{
    Status ret = STATUS_OK;
    ret = m_curDepthStream.create(m_curSensor, SENSOR_DEPTH);
    if (STATUS_OK != ret)
    {
        cout << "Couldn't find depth stream:" << OpenNI::getExtendedError() << endl;
        return -1;
    }
    else
    {
        ret = m_curDepthStream.start();
        if (STATUS_OK != ret)
        {
            cout << "Couldn't start depth stream:" << OpenNI::getExtendedError() << endl;
            m_curDepthStream.destroy();
            return -2;
        }
    }

    if (!m_curDepthStream.isValid())
    {
        cout << "No valid depth streams." << OpenNI::getExtendedError() << endl;
        m_curDepthStream.destroy();
        return -3;
    }

    m_curDepthVideoMode = m_curDepthStream.getVideoMode();
    m_curDepthReso.w = m_curDepthVideoMode.getResolutionX();
    m_curDepthReso.h = m_curDepthVideoMode.getResolutionY();
    m_pDepthARGB = new uchar[4 * m_curDepthReso.w * m_curDepthReso.h];

    if (m_curColorReso.w == m_curDepthReso.w &&
            m_curColorReso.h == m_curDepthReso.h)
    {
        m_width = m_curDepthReso.w;
        m_height = m_curDepthReso.h;
    }
    else
    {
        cout << "Expect color and depth to be in same resolution." << endl;
    }

    m_nTexMapX = m_width;//MIN_CHUNKS_SIZE(width, TEXTURE_SIZE);
    m_nTexMapY = m_height;//MIN_CHUNKS_SIZE(height, TEXTURE_SIZE);
    m_pTexMap = new RGB888Pixel[m_nTexMapX * m_nTexMapY];

    return 0;
}

void Sensors::stopCurDepth()
{
    if (m_curDepthStream.isValid())
    {
        delete[] m_pDepthARGB;
        delete[] m_pTexMap;
        m_curDepthStream.destroy();
    }
}

VideoMode Sensors::getCurDepthVideoMode()
{
    return m_curDepthVideoMode;
}

Resolution_S Sensors::getCurDepthReso()
{
    return m_curDepthReso;
}

uchar* Sensors::getCurDepthData()
{
    m_curDepthStream.readFrame(&m_curDepthFrame);
    const DepthPixel* pDepth = (const DepthPixel*)m_curDepthFrame.getData();
    unsigned int iSize = m_curDepthReso.w * m_curDepthReso.h;
    DepthPixel tMax = *pDepth;

    for(unsigned int i = 1; i < iSize; ++ i) 
	{
        if(pDepth[i] > tMax)
            tMax = pDepth[i];
    }

    int idx = 0;
    for(unsigned int i = 1; i < iSize; ++ i) 
	{
        if((*pDepth) != 0) 
		{
#if 0
            pDepthARGB[idx++] = 0;                             // Blue
            pDepthARGB[idx++] = 255 * (tMax - *pDepth) / tMax; // Green
            pDepthARGB[idx++] = 255 * *pDepth / tMax;          // Red
            pDepthARGB[idx++] = 255 * (tMax - *pDepth) / tMax; // Alpha
#endif
            m_pDepthARGB[idx++] = 0;                             // Blue
            m_pDepthARGB[idx++] = 255 * (tMax - *pDepth) / tMax; // Green
            m_pDepthARGB[idx++] = 255 * *pDepth / tMax;          // Red
            m_pDepthARGB[idx++] = 150;                           // Alpha
        } 
		else 
		{
            m_pDepthARGB[idx++] = 0;
            m_pDepthARGB[idx++] = 0;
            m_pDepthARGB[idx++] = 0;
            m_pDepthARGB[idx++] = 0;
        }
        ++pDepth;
    }

    return m_pDepthARGB;
}

const DepthPixel* Sensors::getCurDepthValue()
{
    const DepthPixel* pDepth = (const DepthPixel*)m_curDepthFrame.getData();
    return pDepth;
}

void Sensors::toggleDepthMirror(int state)
{
    m_curDepthStream.setMirroringEnabled(!m_curDepthStream.getMirroringEnabled());
}

void Sensors::convertProjectiveToWorld(int u, int v, int z, float& px, float& py, float& pz)
{
    float ifx, ify;
    ifx = 1. / m_curSensorParams.l_intr_p[0];
    ify = 1. / m_curSensorParams.l_intr_p[1];

    float tx = (u - m_curSensorParams.l_intr_p[2]) * ifx;
    float ty = (v - m_curSensorParams.l_intr_p[3]) * ify;
    float x0 = tx;
    float y0 = ty;

#if 0
    if (use_distort_coef)
    {
        for (int j = 0; j < 5; j++)
        {
            double r2 = tx * tx + ty * ty;
            double icdist = (1) / (1 + ((m_curSensorParams.l_k[4] * r2 + m_curSensorParams.l_k[1])*r2 + m_curSensorParams.l_k[0])*r2);
            double deltaX = 2 * m_curSensorParams.l_k[2] * tx*ty + m_curSensorParams.l_k[3] * (r2 + 2 * tx*tx);
            double deltaY = m_curSensorParams.l_k[2] * (r2 + 2 * ty*ty) + 2 * m_curSensorParams.l_k[3] * tx*ty;
            tx = (x0 - deltaX)*icdist;
            ty = (y0 - deltaY)*icdist;
        }
    }
#endif

    px = z * tx;
    py = -z * ty;
    pz = z;
}

void Sensors::convertWorldToProjective(float x, float y, float z, float& u, float& v)
{
    float tx = x / z;
    float ty = y / z;

#if 0
    if (use_distort_coef)
    {
        float r2 = tx*tx + ty*ty;
        float f = 1 + m_curSensorParams.r_k[0] * r2 + m_curSensorParams.r_k[1] * r2*r2 + m_curSensorParams.r_k[4] * r2*r2*r2;
        tx *= f;
        ty *= f;
        float dx = tx + 2 * m_curSensorParams.r_k[2] * tx*ty + m_curSensorParams.r_k[3] * (r2 + 2 * tx*tx);
        float dy = ty + 2 * m_curSensorParams.r_k[3] * tx*ty + m_curSensorParams.r_k[2] * (r2 + 2 * ty*ty);
        tx = dx;
        ty = dy;
    }
#endif

    u = tx * m_curSensorParams.r_intr_p[0] + m_curSensorParams.r_intr_p[2];
    v = ty * m_curSensorParams.r_intr_p[1] + m_curSensorParams.r_intr_p[3];
}

void Sensors::transformPointToPointByOwn(float dst[3], const float src[3])
{
    dst[0] = m_curSensorParams.o_r2l_r[0] * src[0] +
             m_curSensorParams.o_r2l_r[1] * src[1] +
             m_curSensorParams.o_r2l_r[2] * src[2] +
             m_curSensorParams.o_r2l_t[0];
    dst[1] = m_curSensorParams.o_r2l_r[3] * src[0] +
             m_curSensorParams.o_r2l_r[4] * src[1] +
             m_curSensorParams.o_r2l_r[5] * src[2] +
             m_curSensorParams.o_r2l_t[1];
    dst[2] = m_curSensorParams.o_r2l_r[6] * src[0] +
             m_curSensorParams.o_r2l_r[7] * src[1] +
             m_curSensorParams.o_r2l_r[8] * src[2] +
             m_curSensorParams.o_r2l_t[2];
}

void Sensors::transformPointToPointByExternal(float dst[3], const float src[3])
{
    dst[0] = m_curSensorParams.e_r2l_r[0] * src[0] +
             m_curSensorParams.e_r2l_r[1] * src[1] +
             m_curSensorParams.e_r2l_r[2] * src[2] +
             m_curSensorParams.e_r2l_t[0];
    dst[1] = m_curSensorParams.e_r2l_r[3] * src[0] +
             m_curSensorParams.e_r2l_r[4] * src[1] +
             m_curSensorParams.e_r2l_r[5] * src[2] +
             m_curSensorParams.e_r2l_t[1];
    dst[2] = m_curSensorParams.e_r2l_r[6] * src[0] +
             m_curSensorParams.e_r2l_r[7] * src[1] +
             m_curSensorParams.e_r2l_r[8] * src[2] +
             m_curSensorParams.e_r2l_t[2];
}

void Sensors::generatePointCloud(vector<ColorPoint3D_S>& vPointCloud, bool bUseOwnParams, bool bUseExternalParams, int cropX, int cropY)
{
    ColorPoint3D_S point;
    float x = 0.0, y = 0.0, z = 0.0;
    const RGB888Pixel* pColor = (const RGB888Pixel*)m_curColorFrame.getData();
    const DepthPixel* pDepth = (const DepthPixel*)m_curDepthFrame.getData();
    int depthWidth = m_curDepthReso.w;
    int depthHeight = m_curDepthReso.h;
    float src_point[3], dst_point[3];

    if (bParamsFileExist && bUseOwnParams && !bUseExternalParams)
    {
        for (int j = cropY; j < depthHeight - cropY; j++)
        {
            for (int i = cropX; i < depthWidth - cropX; i++)
            {
                convertProjectiveToWorld(i, j, pDepth[j * depthWidth + i], src_point[0], src_point[1], src_point[2]);
                transformPointToPointByOwn(dst_point, src_point);
                point.x = 0.001 * dst_point[0];
                point.y = 0.001 * dst_point[1];
                point.z = 0.001 * dst_point[2];
                point.r = pColor[j * depthWidth + i].r;
                point.g = pColor[j * depthWidth + i].g;
                point.b = pColor[j * depthWidth + i].b;
                vPointCloud.push_back(point);
            }
        }
    }
    else if (bParamsFileExist && !bUseOwnParams && bUseExternalParams)
    {
        for (int j = cropY; j < depthHeight - cropY; j++)
        {
            for (int i = cropX; i < depthWidth - cropX; i++)
            {
                convertProjectiveToWorld(i, j, pDepth[j * depthWidth + i], src_point[0], src_point[1], src_point[2]);
                transformPointToPointByExternal(dst_point, src_point);
                point.x = 0.001 * dst_point[0];
                point.y = 0.001 * dst_point[1];
                point.z = 0.001 * dst_point[2];
                point.r = pColor[j * depthWidth + i].r;
                point.g = pColor[j * depthWidth + i].g;
                point.b = pColor[j * depthWidth + i].b;
                vPointCloud.push_back(point);
            }
        }
    }
    else
    {
        for (int j = cropY; j < depthHeight - cropY; j++)
        {
            for (int i = cropX; i < depthWidth - cropX; i++)
            {
                CoordinateConverter::convertDepthToWorld(m_curDepthStream, i, j, pDepth[j * depthWidth + i], &x, &y, &z);
                point.x = 0.001 * x;
                point.y = 0.001 * y;
                point.z = 0.001 * z;
                point.r = pColor[j * depthWidth + i].r;
                point.g = pColor[j * depthWidth + i].g;
                point.b = pColor[j * depthWidth + i].b;
                vPointCloud.push_back(point);
            }
        }
    }
}

int Sensors::initChangedSensor(const char* uri)
{
    Status ret = STATUS_OK;
    ret = m_curSensor.open(uri);
    if (STATUS_OK != ret)
    {
        cout << "Device open failed : " << OpenNI::getExtendedError() << endl;
        return -1;
    }

    parseParamsFile();

    return 0;
}
