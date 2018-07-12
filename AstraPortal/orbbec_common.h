#ifndef ORBBEC_COMMON_H
#define ORBBEC_COMMON_H

#include <QMessageLogger>
#include <OpenNI.h>

using namespace openni;

#define MIN_NUM_CHUNKS(data_size, chunk_size)	((((data_size)-1) / (chunk_size) + 1))
#define MIN_CHUNKS_SIZE(data_size, chunk_size)	(MIN_NUM_CHUNKS(data_size, chunk_size) * (chunk_size))

#define TEXTURE_SIZE	512

typedef struct Point3D_S {
    float x;
    float y;
    float z;
} Point3D_S;

typedef struct ColorPoint3D_S {
    float x;
    float y;
    float z;
    float r;
    float g;
    float b;
} ColorPoint3D_S;

typedef struct Point2D_S {
    int x;
    int y;
} Point2D_S;

typedef struct Resolution_S {
    short w;
    short h;
} Resolution_S;

typedef struct SensorInfo_S {
    char serialNum[12];
    DeviceInfo deviceInfo;
    VideoMode colorVideoMode;
    VideoMode depthVideoMode;
} SensorInfo_S;

typedef struct SensorParams_S {
    float l_intr_p[4];
    float r_intr_p[4];
    float o_r2l_r[9];
    float o_r2l_t[3];
    float e_r2l_r[9];
    float e_r2l_t[3];
} SensorParams_S;

#endif // ORBBEC_COMMON_H
