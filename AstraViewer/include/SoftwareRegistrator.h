#pragma once

#include "opencv/cv.h"
#include "Eigen/Dense"
#include "RGBDCamera.h"

using Eigen::MatrixXd;

#define MAT_NUM          16

class SoftwareRegistrator
{
public:
    SoftwareRegistrator();
    ~SoftwareRegistrator();

	// intrinc
    float fxl, fyl, cxl, cyl; // left fx, fy, cx, cy
    float fxr, fyr, cxr, cyr; // right fx, fy, cx, cy
    
	// extrinc
	float rot[9]; // rotation matrix
    float t[3]; // translation vector
	
	// distortion
	float k1l, k2l, p1l, p2l, k3l; // left k1, k2, p1, p2, k3
	float k1r, k2r, p1r, p2r, k3r; // right k1, k2, p1, p2, k3

	OBCameraParams cameraParams;

	void LoadParamsFromIniFile(char * uri);
	void SetParams(OBCameraParams& params);
	void CalcConversionMatrix();
	// Convert depth from projective to world, in IR coordinate
	void ConvertProjectiveToWorld(int u, int v, int z, float& px, float& py, float& pz, bool use_distort_coef);
	
	// Convert depth from world to projective, in RGB coordinate
	void ConvertWorldToProjective(float x, float y, float z, float& u, float& v, bool use_distort_coef);
	
	// Transform depth point cloud from IR coordinate to RGB coordinate
	void TransformPointToPoint(float dst[3], const float src[3]); 

	void MappingDepth2Color(cv::Mat &src, cv::Mat &dst, bool use_distort_coef);
	void MappingColor2Depth(cv::Mat &CalibDepht, cv::Mat &src, cv::Mat &dst);

private:
	double  c2d[MAT_NUM]; // Color to depth conversion matrix
	double  d2c[MAT_NUM]; // Depth to color conversion matrix
};
