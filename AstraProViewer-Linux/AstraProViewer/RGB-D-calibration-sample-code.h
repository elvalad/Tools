

#ifndef RGB_D_CALIBRATION_SAMPLE_CODE_H
#define RGB_D_CALIBRATION_SAMPLE_CODE_H

#include <opencv2/core/core.hpp>
#include <eigen3/Eigen/Dense>

using Eigen::MatrixXd;


class CameraParameter
{
public:
    CameraParameter();
    ~CameraParameter();

    float fxl, fyl, cxl, cyl;
    float fxr, fyr, cxr, cyr;
    float rot[9];
    float t[3];

    void loadCameraParams(char * uri);

private:

};


typedef struct CameraParams
{
    Eigen::Matrix<double, 1, 4>  left_camera_intrinsic_params;
    Eigen::Matrix<double, 1, 4>  right_camera_intrinsic_params;
    Eigen::Matrix<double, 3, 3>  right2left_rotation_matrix;
    Eigen::Matrix<double, 1, 3>  right2left_translation_vector;
}CameraParams;


void CalcALLMat(float *mat, CameraParameter& camera);

void MappingDepth2Color(cv::Mat &src, cv::Mat &dst, CameraParams &cam, const float *mat);
void MappingDepth2Color(cv::Mat &src, cv::Mat &dst, const float *mat);

#endif
