

#include "RGBDCamera.h"


CRGBDCamera::CRGBDCamera(int width, int height)
{
    m_W = width;
    m_H = height;
}


CRGBDCamera::~CRGBDCamera()
{
}


int CRGBDCamera::InitializeDevice()
{
    //initialize OpenNI
    openni::Status rc = openni::OpenNI::initialize();
    if (rc != openni::STATUS_OK)
    {
        printf("Initialize failed\n%s\n", openni::OpenNI::getExtendedError());
        return -1;
    }   

	return 0;
}

int CRGBDCamera::OpenDevice(const char* uri)
{
    openni::Status rc = openni::STATUS_OK;

    if (m_device.isValid())
    {
        m_device.close();
    }

    rc = m_device.open(uri);
    if (rc != openni::STATUS_OK)
    {
        printf("Couldn't open device\n%s\n", openni::OpenNI::getExtendedError());
        return 02;
    }

	m_device.setDepthColorSyncEnabled(true);

	return 0;
}


int CRGBDCamera::CloseDevice()
{
    m_device.close();
    openni::OpenNI::shutdown();
    return 0;
}

int CRGBDCamera::InitializeRGBorIRStream(bool rgbMode, int rec_h, int rec_w)
{
    openni::Status rc;
    if (m_rgb_ir.isValid())
    {
        m_rgb_ir.stop();
        m_rgb_ir.destroy();
    }

    if (m_depth.isValid())
    {
        m_depth.stop();
        m_depth.destroy();
    }


    if (rgbMode)
    {
        if (m_device.getSensorInfo(openni::SENSOR_COLOR) != NULL)
        {
            rc = m_rgb_ir.create(m_device, openni::SENSOR_COLOR);
            if (rc != openni::STATUS_OK)
            {
                printf("Couldn't create color stream\n%s\n", openni::OpenNI::getExtendedError());
                return 3;
            }
        }
        m_rgb_ir_vmod = m_rgb_ir.getVideoMode();
        m_rgb_ir_vmod.setResolution(rec_w, rec_h);
        m_rgb_ir.setVideoMode(m_rgb_ir_vmod);
        m_ir = 0;
    }
    else//ir
    {
        if (m_device.getSensorInfo(openni::SENSOR_IR) != NULL)
        {
            rc = m_rgb_ir.create(m_device, openni::SENSOR_IR);
            if (rc != openni::STATUS_OK)
            {
                printf("Couldn't create ir stream\n%s\n", openni::OpenNI::getExtendedError());
                return 3;
            }
        }
        m_rgb_ir_vmod = m_rgb_ir.getVideoMode();
        m_rgb_ir_vmod.setPixelFormat(openni::PIXEL_FORMAT_GRAY16);
        m_rgb_ir_vmod.setResolution(rec_w, rec_h);
        m_rgb_ir.setVideoMode(m_rgb_ir_vmod);
        m_ir = 1;
    }

    // Depth
    if (m_device.getSensorInfo(openni::SENSOR_DEPTH) != NULL)
    {
        rc = m_depth.create(m_device, openni::SENSOR_DEPTH);
        if (rc != openni::STATUS_OK)
        {
            printf("Couldn't create depth stream\n%s\n", openni::OpenNI::getExtendedError());
            return 3;
        }
    }
    m_depth_vmod = m_depth.getVideoMode();
    m_depth_vmod.setPixelFormat(openni::PIXEL_FORMAT_DEPTH_1_MM);
    m_depth_vmod.setResolution(rec_w, rec_h);
    m_depth.setVideoMode(m_depth_vmod);

	return 0;
}


int CRGBDCamera::StartRGBorIRStream()
{
    openni::Status rc = m_rgb_ir.start();
    if (rc != openni::STATUS_OK)
    {
        printf("Couldn't start the rgb stream\n%s\n", openni::OpenNI::getExtendedError());
        return 4;
    }

	return 0;
}


int CRGBDCamera::StartDepthStream()
{
    openni::Status rc = m_depth.start();
    if (rc != openni::STATUS_OK)
    {
        printf("Couldn't start the Depth stream\n%s\n", openni::OpenNI::getExtendedError());
        return 4;
    }

	return 0;
}


int CRGBDCamera::StopRGBorIRStream()
{
    m_rgb_ir.stop();
    m_rgb_ir.destroy();
    return 0;
}


int CRGBDCamera::StopDepthStream()
{
    m_depth.stop();
    m_depth.destroy();
    return 0;
}


int CRGBDCamera::IRConvertWORD2BYTE(cv::Mat &src, cv::Mat &dst)
{
    uint16_t H, W;
    W = src.cols; H = src.rows;
    for (unsigned int y = 0; y < H; ++y)
    {
        ushort* data = (ushort*)src.ptr<uchar>(y);
        uchar* val = (uchar*)dst.ptr<uchar>(y);
        for (unsigned int x = 0; x < W; ++x)
        {
            *val = (uchar)((*data) >> 2);
            val++;
            *val = (uchar)((*data) >> 2);
            val++;
            *val = (uchar)((*data) >> 2);
            val++;
            data++;
        }
    }
    return 0;
}




int CRGBDCamera::getNextFrame(cv::Mat& color, cv::Mat& ir)
{
    int changedStreamDummy;
    openni::VideoStream* pStream = &m_rgb_ir;
    openni::Status rc = openni::OpenNI::waitForAnyStream(&pStream, 1, &changedStreamDummy, SAMPLE_READ_WAIT_TIMEOUT);
    

    rc = m_rgb_ir.readFrame(&m_rgb_ir_frame);
    if (rc != openni::STATUS_OK&&m_rgb_ir_frame.isValid())
    {
        printf("Read failed!\n%s\n", openni::OpenNI::getExtendedError());
        return -1;
    }
    if (m_ir == 0)//color
    {
        double resizeFactor = std::min((m_W / (double)m_rgb_ir_frame.getWidth()), (m_H / (double)m_rgb_ir_frame.getHeight()));
        unsigned int texture_x = (unsigned int)(m_W - (resizeFactor * m_rgb_ir_frame.getWidth())) / 2;
        unsigned int texture_y = (unsigned int)(m_H - (resizeFactor * m_rgb_ir_frame.getHeight())) / 2;
        for (unsigned int y = 0; y < (m_H - 2 * texture_y); ++y)
        {
            uint8_t* data = (uint8_t*)color.ptr<uchar>(y);
            for (unsigned int x = 0; x < (m_W - 2 * texture_x); ++x)
            {
                OniRGB888Pixel* streamPixel = (OniRGB888Pixel*)((char*)m_rgb_ir_frame.getData() + ((int)(y / resizeFactor) * m_rgb_ir_frame.getStrideInBytes())) + (int)(x / resizeFactor);
                memcpy(data, streamPixel, 3);
                data = data + 3;
            }
        }
    }
    else//ir
    {
        uint16_t *pPixel;
        for (int y = 0; y < m_H; y++)
        {
            pPixel = ((uint16_t*)((char*)m_rgb_ir_frame.getData() + ((int)(y)* m_rgb_ir_frame.getStrideInBytes())));
            ushort* data = (ushort*)ir.ptr<uchar>(y);
            for (int x = 0; x < m_W; x++)
            {
                *data++ = (*pPixel);
                pPixel++;
            }
        }
    }

    return 0;

}


int CRGBDCamera::getNextFrame(cv::Mat& color, cv::Mat& ir, cv::Mat& depth)
{
    int changedStreamDummy;
    openni::VideoStream* pStream = &m_rgb_ir;
    openni::Status rc = openni::OpenNI::waitForAnyStream(&pStream, 1, &changedStreamDummy, SAMPLE_READ_WAIT_TIMEOUT);


    rc = m_rgb_ir.readFrame(&m_rgb_ir_frame);
    if (rc != openni::STATUS_OK && m_rgb_ir_frame.isValid())
    {
        printf("Read failed!\n%s\n", openni::OpenNI::getExtendedError());
        return -1;
    }
    if (m_ir == 0)//color
    {
        double resizeFactor = std::min((m_W / (double)m_rgb_ir_frame.getWidth()), (m_H / (double)m_rgb_ir_frame.getHeight()));
        unsigned int texture_x = (unsigned int)(m_W - (resizeFactor * m_rgb_ir_frame.getWidth())) / 2;
        unsigned int texture_y = (unsigned int)(m_H - (resizeFactor * m_rgb_ir_frame.getHeight())) / 2;
        for (unsigned int y = 0; y < (m_H - 2 * texture_y); ++y)
        {
            uint8_t* data = (uint8_t*)color.ptr<uchar>(y);
            for (unsigned int x = 0; x < (m_W - 2 * texture_x); ++x)
            {
                OniRGB888Pixel* streamPixel = (OniRGB888Pixel*)((char*)m_rgb_ir_frame.getData() + ((int)(y / resizeFactor) * m_rgb_ir_frame.getStrideInBytes())) + (int)(x / resizeFactor);
                memcpy(data, streamPixel, 3);
                data = data + 3;
            }
        }
    }
    else//ir
    {
        uint16_t *pPixel;
        for (int y = 0; y < m_H; y++)
        {
            pPixel = ((uint16_t*)((char*)m_rgb_ir_frame.getData() + ((int)(y)* m_rgb_ir_frame.getStrideInBytes())));
            ushort* data = (ushort*)ir.ptr<uchar>(y);
            for (int x = 0; x < m_W; x++)
            {
                *data++ = (*pPixel);
                pPixel++;
            }
        }
    }


    openni::VideoStream* pDepthStream = &m_depth;
    rc = openni::OpenNI::waitForAnyStream(&pDepthStream, 1, &changedStreamDummy, SAMPLE_READ_WAIT_TIMEOUT);

    rc = m_depth.readFrame(&m_depth_frame);
    if (rc != openni::STATUS_OK || !m_depth_frame.isValid())
    {
        printf("Read failed!\n%s\n", openni::OpenNI::getExtendedError());
        return -1;
    }
   
        uint16_t *pPixel;
        for (int y = 0; y < m_H; y++)
        {
            pPixel = ((uint16_t*)((char*)m_depth_frame.getData() + ((int)(y)* m_depth_frame.getStrideInBytes())));
            uint16_t* data = (uint16_t*)depth.ptr<uchar>(y);
            for (int x = 0; x < m_W; x++)
            {
                *data++ = (*pPixel);
                pPixel++;

                /*if (y == m_H / 2 && x == m_W / 2)
                {
                    std::cout << " depth = " << *pPixel << std::endl;
                }*/
            }      
        }
    return 0;
}

bool  CRGBDCamera::toggleRegister(bool setRegister)
{
    if (m_device.isImageRegistrationModeSupported(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR))
    {
        if (setRegister){
            m_device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR);
        }
        else{
            m_device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_OFF);
        }

        return true;
    }
    else
    {
        return false;
    }
}


int CRGBDCamera::LoadCameraParams(OBCameraParams& params)
{
	int size = sizeof(OBCameraParams);
	m_device.getProperty(openni::OBEXTENSION_ID_CAM_PARAMS, (uint8_t*)&params, &size);
	printf("fx_l : %f, fy_l : %f, cx_l : %f, cy_l : %f\n", params.l_intr_p[0], params.l_intr_p[1], params.l_intr_p[2], params.l_intr_p[3]);
	printf("fx_r : %f, fy_r : %f, cx_r : %f, cy_r : %f\n", params.r_intr_p[0], params.r_intr_p[1], params.r_intr_p[2], params.r_intr_p[3]);
	printf("r[0] : %f, r[1] : %f, r[2] : %f\n", params.r2l_r[0], params.r2l_r[1], params.r2l_r[2]);
	printf("r[3] : %f, r[4] : %f, r[5] : %f\n", params.r2l_r[3], params.r2l_r[4], params.r2l_r[5]);
	printf("r[6] : %f, r[7] : %f, r[8] : %f\n", params.r2l_r[6], params.r2l_r[7], params.r2l_r[8]);
	printf("t[0] : %f, t[1] : %f, t[2] : %f\n", params.r2l_t[0], params.r2l_t[1], params.r2l_t[2]);
	printf("lk[0] : %f, lk[1] : %f, lk[2] : %f, lk[3] : %f, lk[4] : %f\n", params.l_k[0], params.l_k[1], params.l_k[2], params.l_k[3], params.l_k[4]);
	printf("rk[0] : %f, rk[1] : %f, rk[2] : %f, rk[3] : %f, rk[4] : %f\n", params.r_k[0], params.r_k[1], params.r_k[2], params.r_k[3], params.r_k[4]);
	return 0;
}
