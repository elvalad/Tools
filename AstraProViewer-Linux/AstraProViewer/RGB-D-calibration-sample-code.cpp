
#include <stdint.h>
#include <iostream>
#include <fstream>

#include "RGB-D-calibration-sample-code.h"

CameraParameter::CameraParameter()
{

}

CameraParameter::~CameraParameter()
{

}

void CameraParameter::loadCameraParams(char * uri)
{
    char tmpbuf[128];
    float cx, cy, fx, fy, zero;
    float tx, ty, tz;
    std::ifstream inf;

    inf.open(uri, std::ios::in);
    inf.getline(tmpbuf, sizeof(tmpbuf));
    inf >> fxl >> zero >> cxl >>
        zero >> fyl >> cyl >>
        zero >> zero >> zero;

    inf.getline(tmpbuf, sizeof(tmpbuf));
    inf.getline(tmpbuf, sizeof(tmpbuf));
    inf >> fxr >> zero >> cxr >>
        zero >> fyr >> cyr >>
        zero >> zero >> zero;

    inf.getline(tmpbuf, sizeof(tmpbuf));
    inf.getline(tmpbuf, sizeof(tmpbuf));
    inf >> zero; rot[0] = zero;
    inf >> zero; rot[1] = zero;
    inf >> zero; rot[2] = zero;
    inf >> zero; rot[3] = zero;
    inf >> zero; rot[4] = zero;
    inf >> zero; rot[5] = zero;
    inf >> zero; rot[6] = zero;
    inf >> zero; rot[7] = zero;
    inf >> zero; rot[8] = zero;

    inf.getline(tmpbuf, sizeof(tmpbuf));
    inf.getline(tmpbuf, sizeof(tmpbuf));
    inf >> tx; t[0] = tx;
    inf >> ty; t[1] = ty;
    inf >> tz; t[2] = tz;

    inf.close();
}


void CalcALLMat(float *mat, CameraParameter& camera)
{
    Eigen::Matrix<double, 4, 4> R2L_Mat;
    Eigen::Matrix<double, 4, 4> RK_Mat;
    Eigen::Matrix<double, 4, 4> LK_Mat;
    Eigen::Matrix<double, 4, 4> iLK_Mat;
    Eigen::Matrix<double, 4, 4> All_Mat;

    RK_Mat << camera.fxr, 0.0, camera.cxr, 0.0,
        0.0, camera.fyr, camera.cyr, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0; 

    R2L_Mat << camera.rot[0], camera.rot[1], camera.rot[2], camera.t[0],
        camera.rot[3], camera.rot[4], camera.rot[5], camera.t[1],
        camera.rot[6], camera.rot[7], camera.rot[8], camera.t[2],
               0.0, 0.0, 0.0, 1.0;

    LK_Mat << camera.fxl, 0.0, camera.cxl, 0.0,
        0.0, camera.fyl, camera.cyl, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0;

    All_Mat = RK_Mat*R2L_Mat*LK_Mat.inverse();
    mat[0] = All_Mat(0, 0);
    mat[1] = All_Mat(0, 1);
    mat[2] = All_Mat(0, 2);
    mat[3] = All_Mat(0, 3);
    mat[4] = All_Mat(1, 0);
    mat[5] = All_Mat(1, 1);
    mat[6] = All_Mat(1, 2);
    mat[7] = All_Mat(1, 3);
    mat[8] = All_Mat(2, 0);
    mat[9] = All_Mat(2, 1);
    mat[10] = All_Mat(2, 2);
    mat[11] = All_Mat(2, 3);
    mat[12] = All_Mat(3, 0);
    mat[13] = All_Mat(3, 1);
    mat[14] = All_Mat(3, 2);
    mat[15] = All_Mat(3, 3);

    std::cout << "RK_Mat" << std::endl << RK_Mat << std::endl;
    std::cout << "LK_Mat" << std::endl << LK_Mat << std::endl;
    std::cout << "R2L_Mat" << std::endl << R2L_Mat << std::endl;
    std::cout << "All_Mat" << std::endl << All_Mat << std::endl;
}

void MappingDepth2Color(cv::Mat &src, cv::Mat &dst,const float *mat)
{
    double  z;
    uint16_t u, v,d;
    uint16_t u_rgb, v_rgb;
    cv::Mat newdepth(dst.rows, dst.cols, CV_16UC1, cv::Scalar(0));
    for (v = 0; v < src.rows; v++)
    {
        for (u = 0; u < src.cols; u++)
        {
            d = src.at<uint16_t>(v, u);
            //printf("%d,%d,%d\r\n",u,v,d);
            z = (double)d;
            u_rgb = (uint16_t)((mat[0] * (double)u + mat[1] * (double)v + mat[2] + mat[3] / z));
            v_rgb = (uint16_t)((mat[4] * (double)u + mat[5] * (double)v + mat[6] + mat[7] / z));

            if (u_rgb < 0 || u_rgb >= newdepth.cols || v_rgb < 0 || v_rgb >= newdepth.rows) continue;
            uint16_t *val = (uint16_t *)newdepth.ptr<uchar>(v_rgb)+u_rgb;
            *val = d;
        }
    }
    dst = newdepth;
}
