#include "RGBDCamera.h"


struct  _device_vid_pid
{
    uint16_t vid;
    uint16_t pid;
};

static struct _device_vid_pid support_devices_list[] =
{
    { 0x2BC5, 0x0400 },
    { 0x2BC5, 0x0401 },
    { 0x2BC5, 0x0402 },
    { 0x2BC5, 0x0403 },
    { 0x2BC5, 0x0404 },
    { 0x2BC5, 0x0405 },
    { 0x2BC5, 0x04FF }
};

CRGBDCamera::CRGBDCamera()
{
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

    int status = xnUSBInit();
    if (status != 0)
    {
        return -1;
    }
}

int CRGBDCamera::OpenDevice()
{
    openni::Status rc = openni::STATUS_OK;

    rc = m_device.open(openni::ANY_DEVICE);
    if (rc != openni::STATUS_OK)
    {
        printf("Couldn't open device\n%s\n", openni::OpenNI::getExtendedError());
        return 02;
    }

    
}

char* CRGBDCamera::getSerialNumber()
{   
    int SN_size = 13;
    m_device.getProperty((int)ONI_DEVICE_PROPERTY_SERIAL_NUMBER, (void *)m_SerialNumber, &SN_size);

    return m_SerialNumber;
}

int CRGBDCamera::CloseDevice()
{
    m_device.close();
    OpenNI::shutdown();
    return 0;
}

int CRGBDCamera::InitializeRGBorIRStream(bool rgbMode, int ir_h, int ir_w)
{
    openni::Status rc;
    if (m_rgb_ir.isValid())
    {
        m_rgb_ir.stop();
        m_rgb_ir.destroy();
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
        m_rgb_ir_vmod.setResolution(ir_w, ir_h);
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
        m_rgb_ir_vmod.setResolution(1280, 1024);
        m_rgb_ir.setVideoMode(m_rgb_ir_vmod);
        m_ir = 1;
    }

}


int CRGBDCamera::StartRGBorIRStream()
{
    openni::Status rc = m_rgb_ir.start();
    if (rc != openni::STATUS_OK)
    {
        printf("Couldn't start the rgb stream\n%s\n", openni::OpenNI::getExtendedError());
        return 4;
    }
}


int CRGBDCamera::StopRGBorIRStream()
{
    m_rgb_ir.stop();
    m_rgb_ir.destroy();
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




int CRGBDCamera::getNextFrame(cv::Mat& color, cv::Mat& ir, cv::Mat& yuv)
{
    int changedStreamDummy;
    openni::VideoStream* pStream = &m_depth;
    openni::Status rc = openni::OpenNI::waitForAnyStream(&pStream, 1, &changedStreamDummy, SAMPLE_READ_WAIT_TIMEOUT);
    

    rc = m_rgb_ir.readFrame(&m_frame);
    if (rc != openni::STATUS_OK&&m_frame.isValid())
    {
        printf("Read failed!\n%s\n", openni::OpenNI::getExtendedError());
        return -1;
    }
    if (m_ir == 0)//color
    {
        int m_W = ir.cols;
        int m_H = ir.rows;
        double resizeFactor = std::min((m_W / (double)m_frame.getWidth()), (m_H / (double)m_frame.getHeight()));
        unsigned int texture_x = (unsigned int)(m_W - (resizeFactor * m_frame.getWidth())) / 2;
        unsigned int texture_y = (unsigned int)(m_H - (resizeFactor * m_frame.getHeight())) / 2;
        for (unsigned int y = 0; y < (m_H - 2 * texture_y); ++y)
        {
            uint8_t* data_rgb = (uint8_t*)color.ptr<uchar>(y);
            uint8_t* data_yuv = (uint8_t*)yuv.ptr<uchar>(y);
            for (unsigned int x = 0; x < (m_W - 2 * texture_x); ++x)
            {
                OniRGB888Pixel* streamPixel = (OniRGB888Pixel*)((char*)m_frame.getData() + ((int)(y / resizeFactor) * m_frame.getStrideInBytes())) + (int)(x / resizeFactor);
                memcpy(data_rgb, streamPixel, 3);

                uint8_t r = streamPixel->r;
                uint8_t g = streamPixel->g;
                uint8_t b = streamPixel->b;
                uint8_t yuv = 0.299 * r + 0.587 * g + 0.114 * b;
                streamPixel->r = yuv;
                streamPixel->g = yuv;
                streamPixel->b = yuv;
                memcpy(data_yuv, streamPixel, 3);

                data_yuv = data_yuv + 3;
                data_rgb = data_rgb + 3;
            }
        }
    }
    else//ir
    {
        int IR_W = ir.cols;
        int IR_H = ir.rows;
        uint16_t *pPixel;

        unsigned int y = 0;
        int ystart = 0;
        int ystep = 1;
        int yend = 0;
        int yScale = 1;

        if (IR_W == 640)
        {
            y = 8;
            ystart = 8;
            ystep = 2;
            yend = 56;
            yScale = 2;
        }

        for (; y < 1024 - yend; y = y + ystep)
        {
            pPixel = ((uint16_t*)((char*)m_frame.getData() + ((int)(y)* m_frame.getStrideInBytes())));
            ushort* data = (ushort*)ir.ptr<uchar>((y - ystart) / yScale);
            for (unsigned int x = 0; x < 1280; x = x + ystep)
            {
                *data++ = (*pPixel);
                pPixel++;
                if (IR_W == 640)
                {
                    pPixel++;
                }
            }
        }
        /*for (unsigned int y = 8; y < 1024 - 56; y = y + 2)
        {
            pPixel = ((uint16_t*)((char*)m_frame.getData() + ((int)(y)* m_frame.getStrideInBytes())));
            ushort* data = (ushort*)ir.ptr<uchar>((y - 8) / 2);
            for (unsigned int x = 0; x < 1280; x = x + 2)
            {
                *data++ = (*pPixel);
                pPixel++;
                pPixel++;
            }
        }*/
    }

    return 0;

}


int CRGBDCamera::EnumDevice(uint16_t &vid, uint16_t &pid)
{
    int rc = 0;
    uint32_t n = 0;
    for (int i = 0; i < sizeof(support_devices_list) / sizeof(struct _device_vid_pid); i++)
    {
        rc = xnUSBEnumerateDevices(support_devices_list[i].vid, support_devices_list[i].pid, &m_astrDevicePaths, &n);
        if ((rc == 0 && n != 0))
        {
            m_vid = support_devices_list[i].vid;
            m_pid = support_devices_list[i].pid;

            vid = m_vid;
            pid = m_pid;

            break;
        }
    }
    if ((rc != 0) || (n == 0))
    {
        return -1;
    }
    return 0;
}