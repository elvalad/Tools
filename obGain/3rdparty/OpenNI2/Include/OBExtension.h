#ifndef _OBEXTENSION_H_
#define _OBEXTENSION_H_

#include <OpenNI.h> 



#define EXTENSION_INFO_GET    0x00000001
#define EXTENSION_INFO_SET    0x00000002
#define EXTENSION_END         0xFFFFFFFF      

typedef  void * OB_USB_HANDLE;


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


struct OBExtensionCommand
{
	uint32_t id;
	uint32_t info;
	uint32_t datasize;//byte
};


enum OBExternsionID
{
    OBEXTENSION_ID_IR_GAIN = 0,
    OBEXTENSION_ID_LDP_EN,
    OBEXTENSION_ID_CAM_PARAMS,
    OBEXTENSION_ID_LASER_EN,
    OBEXTENSION_ID_SERIALNUMBER,
    OBEXTENSION_ID_DEVICETYPE
};

struct OBExtension
{
   OB_USB_HANDLE m_hUSBDevice;
   uint16_t cam_tag;
   int vid;
   int pid;
};

/*
must be called after  device has been opened
*/
ONI_C_API openni::Status OBExtension_Init(OBExtension *ext, const openni::Device *device);
ONI_C_API openni::Status OBExtension_Deinit(OBExtension *ext);
ONI_C_API openni::Status OBExtension_GetProperty(OBExtension *ext, int id, void* data, int dataSize);
ONI_C_API openni::Status OBExtension_SetProperty(OBExtension *ext, int id, const void* data, int dataSize);


#endif /*_OBEXTENSION_H_*/
