

#include <stdint.h>
#include <iostream>
#include <fstream>
#include <omp.h>

#include "SoftwareRegistrator.h"

using namespace std;
using namespace cv;

SoftwareRegistrator::SoftwareRegistrator()
{

}

SoftwareRegistrator::~SoftwareRegistrator()
{

}

void SoftwareRegistrator::LoadParamsFromIniFile(char * uri)
{
    char tmpbuf[128];
    float tmp;
    float tx, ty, tz;
    std::ifstream inf;

    inf.open(uri, std::ios::in);

	// Read section [Left Camera Intrinsic]
    inf.getline(tmpbuf, sizeof(tmpbuf)); 

	inf >> fxl >> tmp >> cxl >>
		   tmp >> fyl >> cyl >>
		   tmp >> tmp >> tmp;

	// Read section [Right Camera Intrinsic]
    inf.getline(tmpbuf, sizeof(tmpbuf));
    inf.getline(tmpbuf, sizeof(tmpbuf)); 

    inf >> fxr >> tmp >> cxr >>
           tmp >> fyr >> cyr >>
		   tmp >> tmp >> tmp;

	// Read section [Right to Left Camera Rotate Matrix]
    inf.getline(tmpbuf, sizeof(tmpbuf));
    inf.getline(tmpbuf, sizeof(tmpbuf)); 

	inf >> tmp; rot[0] = tmp;
    inf >> tmp; rot[1] = tmp;
    inf >> tmp; rot[2] = tmp;
    inf >> tmp; rot[3] = tmp;
    inf >> tmp; rot[4] = tmp;
    inf >> tmp; rot[5] = tmp;
    inf >> tmp; rot[6] = tmp;
    inf >> tmp; rot[7] = tmp;
    inf >> tmp; rot[8] = tmp;

	// Read section [Right to Left Camera Translate]
    inf.getline(tmpbuf, sizeof(tmpbuf));
    inf.getline(tmpbuf, sizeof(tmpbuf)); 

    inf >> tx; t[0] = tx;
    inf >> ty; t[1] = ty;
    inf >> tz; t[2] = tz;

	// Read section [IR Camera Distorted Params]
	// k1 k2 k3 p1 p2
	inf.getline(tmpbuf, sizeof(tmpbuf));
	inf.getline(tmpbuf, sizeof(tmpbuf));
	inf >> tmp; k1l = tmp;
	inf >> tmp; k2l = tmp;
	inf >> tmp; k3l = tmp;
	inf >> tmp; p1l = tmp;
	inf >> tmp; p2l = tmp;

	// Read section [RGB Camera Distorted Params]
	// k1 k2 k3 p1 p2
	inf.getline(tmpbuf, sizeof(tmpbuf));
	inf.getline(tmpbuf, sizeof(tmpbuf));
	inf >> tmp; k1r = tmp;
	inf >> tmp; k2r = tmp;
	inf >> tmp; k3r = tmp;
	inf >> tmp; p1r = tmp;
	inf >> tmp; p2r = tmp;

	// Set camera params from file
	cameraParams.l_intr_p[0] = fxl;
	cameraParams.l_intr_p[1] = fyl;
	cameraParams.l_intr_p[2] = cxl;
	cameraParams.l_intr_p[3] = cyl;

	cameraParams.r_intr_p[0] = fxr;
	cameraParams.r_intr_p[1] = fyr;
	cameraParams.r_intr_p[2] = cxr;
	cameraParams.r_intr_p[3] = cyr;

	cameraParams.r2l_r[0] = rot[0];
	cameraParams.r2l_r[1] = rot[1];
	cameraParams.r2l_r[2] = rot[2];
	cameraParams.r2l_r[3] = rot[3];
	cameraParams.r2l_r[4] = rot[4];
	cameraParams.r2l_r[5] = rot[5];
	cameraParams.r2l_r[6] = rot[6];
	cameraParams.r2l_r[7] = rot[7];
	cameraParams.r2l_r[8] = rot[8];

	cameraParams.r2l_t[0] = t[0];
	cameraParams.r2l_t[1] = t[1];
	cameraParams.r2l_t[2] = t[2];

	// k1 k2 p1 p2 k3
	cameraParams.l_k[0] = k1l;
	cameraParams.l_k[1] = k2l;
	cameraParams.l_k[2] = p1l;
	cameraParams.l_k[3] = p2l;
	cameraParams.l_k[4] = k3l;

	// k1 k2 p1 p2 k3
	cameraParams.r_k[0] = k1r;
	cameraParams.r_k[1] = k2r;
	cameraParams.r_k[2] = p1r;
	cameraParams.r_k[3] = p2r;
	cameraParams.r_k[4] = k3r;

    inf.close();
}


void SoftwareRegistrator::SetParams(OBCameraParams& params)
{
	fxl = params.l_intr_p[0];
	cxl = params.l_intr_p[2];
	fyl = params.l_intr_p[1];
	cyl = params.l_intr_p[3];

	fxr = params.r_intr_p[0];
	cxr = params.r_intr_p[2];
	fyr = params.r_intr_p[1];
	cyr = params.r_intr_p[3];

	rot[0] = params.r2l_r[0];
	rot[1] = params.r2l_r[1];
	rot[2] = params.r2l_r[2];
	rot[3] = params.r2l_r[3];
	rot[4] = params.r2l_r[4];
	rot[5] = params.r2l_r[5];
	rot[6] = params.r2l_r[6];
	rot[7] = params.r2l_r[7];
	rot[8] = params.r2l_r[8];

	t[0] = params.r2l_t[0];
	t[1] = params.r2l_t[1];
	t[2] = params.r2l_t[2];

	// Distortion parameter is               k1 k2 p1 p2 k3
	// Astra camera distortion parameter is  k1 k2 k3 p1 p2
	k1l = params.l_k[0];
	k2l = params.l_k[1];
	p1l = params.l_k[3];
	p2l = params.l_k[4];
	k3l = params.l_k[2];

	k1r = params.r_k[0];
	k2r = params.r_k[1];
	p1r = params.r_k[3];
	p2r = params.r_k[4];
	k3r = params.r_k[2];

	cameraParams = params;

	cameraParams.l_k[0] = k1l;
	cameraParams.l_k[1] = k2l;
	cameraParams.l_k[2] = p1l;
	cameraParams.l_k[3] = p2l;
	cameraParams.l_k[4] = k3l;

	cameraParams.r_k[0] = k1r;
	cameraParams.r_k[1] = k2r;
	cameraParams.r_k[2] = p1r;
	cameraParams.r_k[3] = p2r;
	cameraParams.r_k[4] = k3r;
}


void SoftwareRegistrator::CalcConversionMatrix()
{
    Eigen::Matrix<double, 4, 4> R2L_Mat;
    Eigen::Matrix<double, 4, 4> RK_Mat;
    Eigen::Matrix<double, 4, 4> LK_Mat;
    Eigen::Matrix<double, 4, 4> iLK_Mat;
    Eigen::Matrix<double, 4, 4> All_Mat;
    Eigen::Matrix<double, 4, 4> All_Mat_inverse;

	LK_Mat << fxl,  0.0,  cxl,  0.0,
		      0.0,  fyl,  cyl,  0.0,
			  0.0,  0.0,  1.0,  0.0,
			  0.0,  0.0,  0.0,  1.0;

    RK_Mat << fxr,  0.0,  cxr,  0.0,
              0.0,  fyr,  cyr,  0.0,
              0.0,  0.0,  1.0,  0.0,
              0.0,  0.0,  0.0,  1.0; 

    R2L_Mat << rot[0], rot[1], rot[2], t[0],
               rot[3], rot[4], rot[5], t[1],
               rot[6], rot[7], rot[8], t[2],
               0.0,    0.0,    0.0,    1.0;

    All_Mat = RK_Mat * R2L_Mat * LK_Mat.inverse();

    d2c[0] = All_Mat(0, 0);
    d2c[1] = All_Mat(0, 1);
    d2c[2] = All_Mat(0, 2);
    d2c[3] = All_Mat(0, 3);
    d2c[4] = All_Mat(1, 0);
    d2c[5] = All_Mat(1, 1);
    d2c[6] = All_Mat(1, 2);
    d2c[7] = All_Mat(1, 3);
    d2c[8] = All_Mat(2, 0);
    d2c[9] = All_Mat(2, 1);
    d2c[10] = All_Mat(2, 2);
    d2c[11] = All_Mat(2, 3);
    d2c[12] = All_Mat(3, 0);
    d2c[13] = All_Mat(3, 1);
    d2c[14] = All_Mat(3, 2);
    d2c[15] = All_Mat(3, 3);

	//std::cout << "LK_Mat" << std::endl << LK_Mat << std::endl;
	//std::cout << "RK_Mat" << std::endl << RK_Mat << std::endl;
    //std::cout << "R2L_Mat" << std::endl << R2L_Mat << std::endl;
    //std::cout << "All_Mat" << std::endl << All_Mat << std::endl;

    All_Mat_inverse = All_Mat.inverse();
    c2d[0] = All_Mat_inverse(0, 0);
    c2d[1] = All_Mat_inverse(0, 1);
    c2d[2] = All_Mat_inverse(0, 2);
    c2d[3] = All_Mat_inverse(0, 3);
    c2d[4] = All_Mat_inverse(1, 0);
    c2d[5] = All_Mat_inverse(1, 1);
    c2d[6] = All_Mat_inverse(1, 2);
    c2d[7] = All_Mat_inverse(1, 3);
    c2d[8] = All_Mat_inverse(2, 0);
    c2d[9] = All_Mat_inverse(2, 1);
    c2d[10] = All_Mat_inverse(2, 2);
    c2d[11] = All_Mat_inverse(2, 3);
    c2d[12] = All_Mat_inverse(3, 0);
    c2d[13] = All_Mat_inverse(3, 1);
    c2d[14] = All_Mat_inverse(3, 2);
    c2d[15] = All_Mat_inverse(3, 3);
    //std::cout << "All_Mat_inverse" << std::endl << All_Mat_inverse << std::endl;
}


void SoftwareRegistrator::ConvertProjectiveToWorld(int u, int v, int z, float& px, float& py, float& pz, bool use_distort_coef)
{
	// x = (u * z - z * Cx) / fx
	// y = (v * z - z * Cy) / fy

	//x = (u * z - z * cxl) / fxl;
	//y = (v * z - z * cyl) / fyl;

	float ifx, ify;
	ifx = 1. / cameraParams.l_intr_p[0];
	ify = 1. / cameraParams.l_intr_p[1];

	float tx = (u - cameraParams.l_intr_p[2]) * ifx;
	float ty = (v - cameraParams.l_intr_p[3]) * ify;
	float x0 = tx;
	float y0 = ty;

	if (use_distort_coef)
	{
		for (int j = 0; j < 5; j++)
		{
			double r2 = tx * tx + ty * ty;
			double icdist = (1) / (1 + ((cameraParams.l_k[4] * r2 + cameraParams.l_k[1])*r2 + cameraParams.l_k[0])*r2);
			double deltaX = 2 * cameraParams.l_k[2] * tx*ty + cameraParams.l_k[3] * (r2 + 2 * tx*tx);
			double deltaY = cameraParams.l_k[2] * (r2 + 2 * ty*ty) + 2 * cameraParams.l_k[3] * tx*ty;
			tx = (x0 - deltaX)*icdist;
			ty = (y0 - deltaY)*icdist;
		}
	}
	
	px = z * tx;
	py = z * ty;
	pz = z;
}

void SoftwareRegistrator::ConvertWorldToProjective(float x, float y, float z, float& u, float& v, bool use_distort_coef)
{
	float tx = x / z;
	float ty = y / z;
	if (use_distort_coef)
	{
		float r2 = tx*tx + ty*ty;
		float f = 1 + cameraParams.r_k[0] * r2 + cameraParams.r_k[1] * r2*r2 + cameraParams.r_k[4] * r2*r2*r2;
		tx *= f;
		ty *= f;
		float dx = tx + 2 * cameraParams.r_k[2] * tx*ty + cameraParams.r_k[3] * (r2 + 2 * tx*tx);
		float dy = ty + 2 * cameraParams.r_k[3] * tx*ty + cameraParams.r_k[2] * (r2 + 2 * ty*ty);
		tx = dx;
		ty = dy;
	}

	u = tx * cameraParams.r_intr_p[0] + cameraParams.r_intr_p[2];
	v = ty * cameraParams.r_intr_p[1] + cameraParams.r_intr_p[3];
}

void SoftwareRegistrator::TransformPointToPoint(float dst[3], const float src[3])
{
	dst[0] = cameraParams.r2l_r[0] * src[0] + cameraParams.r2l_r[3] * src[1] + cameraParams.r2l_r[6] * src[2] + cameraParams.r2l_t[0];
	dst[1] = cameraParams.r2l_r[1] * src[0] + cameraParams.r2l_r[4] * src[1] + cameraParams.r2l_r[7] * src[2] + cameraParams.r2l_t[1];
	dst[2] = cameraParams.r2l_r[2] * src[0] + cameraParams.r2l_r[5] * src[1] + cameraParams.r2l_r[8] * src[2] + cameraParams.r2l_t[2];
}

void SoftwareRegistrator::MappingDepth2Color(cv::Mat &src, cv::Mat &dst, bool use_distort_coef, bool use_rt_coef, const char* fileName)
{
	ofstream ouF;
	float x_3, y_3, z_3, r_3, g_3, b_3;
	ouF.open(fileName, ofstream::out);
	if (!ouF)
	{
		cerr << "failed to open the file : " << fileName << endl;
		return;
	}
	ouF << "ply" << std::endl;
	ouF << "format ascii 1.0" << std::endl;
	ouF << "comment made by Orbbec " << std::endl;
	ouF << "comment Orbbec Co.,Ltd." << std::endl;
	ouF << "element vertex " << src.rows * src.cols << std::endl;
	ouF << "property float32 x" << std::endl;
	ouF << "property float32 y" << std::endl;
	ouF << "property float32 z" << std::endl;
	ouF << "property uint8 red" << std::endl;
	ouF << "property uint8 green" << std::endl;
	ouF << "property uint8 blue" << std::endl;
	ouF << "element face 0" << std::endl;
	ouF << "property list uint8 int32 vertex_index" << std::endl;
	ouF << "end_header" << std::endl;
	
	cout << src.rows << " " << src.cols << endl;

    double  z;
	float pixel[2];
	float point[3], to_point[3];
    uint16_t u, v, d;
    uint16_t u_rgb = 0, v_rgb = 0;
    cv::Mat newdepth(dst.rows, dst.cols, CV_16UC1, cv::Scalar(0));
    for (v = 0; v < src.rows; v++)
    {
        for (u = 0; u < src.cols; u++)
        {
            d = src.at<uint16_t>(v, u);
            z = (double)d;

			if (use_rt_coef)
			{
				ConvertProjectiveToWorld(u, v, z, point[0], point[1], point[2], use_distort_coef);
				TransformPointToPoint(to_point, point);
				 x_3 = to_point[0];
				 y_3 = to_point[1];
				 z_3 = to_point[2];

				 r_3 = 150;
				 g_3 = 150;
				 b_3 = 150;

				 ouF << x_3 << " " << y_3 << " " << z_3 << " " << r_3 << " " << g_3 << " " << b_3 << std::endl;
			}
			else
			{
				ConvertProjectiveToWorld(u, v, z, point[0], point[1], point[2], use_distort_coef);
				 x_3 = point[0];
				 y_3 = point[1];
				 z_3 = point[2];

				 r_3 = 150;
				 g_3 = 150;
				 b_3 = 150;

				 ouF << x_3 << " " << y_3 << " " << z_3 << " " << r_3 << " " << g_3 << " " << b_3 << std::endl;
			}
			
			u_rgb = (uint16_t)(pixel[0]);
			v_rgb = (uint16_t)(pixel[1]);
            if (u_rgb < 0 || u_rgb >= newdepth.cols || v_rgb < 0 || v_rgb >= newdepth.rows) continue;
            uint16_t *val = (uint16_t *)newdepth.ptr<uchar>(v_rgb)+u_rgb;
            *val = d;
        }
    }

    dst = newdepth;
}

void SoftwareRegistrator::MappingColor2Depth(cv::Mat &CalibDepht, cv::Mat &src, cv::Mat &dst)
{
    double  z;
    uint16_t u, v, d;
    uint16_t u_rgb = 0, v_rgb = 0;
    cv::Mat newcolor(dst.rows, dst.cols, CV_8UC3, cv::Scalar(0));
    for (v = 0; v < src.rows; v++)
    {
        for (u = 0; u < src.cols; u++)
        {   
            d = CalibDepht.at<uint16_t>(v, u);

            z = (double)d;
			u_rgb = (uint16_t)(c2d[0] * (double)u + c2d[1] * (double)v + c2d[2] + c2d[3] / z);
			v_rgb = (uint16_t)(c2d[4] * (double)u + c2d[5] * (double)v + c2d[6] + c2d[7] / z);
            if (u_rgb < 0 || u_rgb >= newcolor.cols || v_rgb < 0 || v_rgb >= newcolor.rows) continue;
            OniRGB888Pixel *val = (OniRGB888Pixel *)newcolor.ptr<uchar>(v_rgb)+u_rgb;
            OniRGB888Pixel *srcColor = (OniRGB888Pixel *)src.ptr<uchar>(v)+u;
            *val = *srcColor ;
        }
    }

    dst = newcolor;
}