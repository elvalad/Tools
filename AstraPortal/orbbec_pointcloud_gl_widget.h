#ifndef ORBBEC_POINTCLOUD_GL_WIDGET_H
#define ORBBEC_POINTCLOUD_GL_WIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>

#include "orbbec_common.h"



class PointcloudGLWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit PointcloudGLWidget(QWidget *parent = 0);
    ~PointcloudGLWidget();
    void updatePointcloud(std::vector<ColorPoint3D_S>& vPointCloud, bool bShowAxis, bool bUseRgbComponent);
    void savePly(QString serialNum);
    void resetPointcloud();

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void qNormalizeAngle(int &angle);

public slots:
    // slots for xyz-rotation slider
    void setXRotation(int angle);
    void setYRotation(int angle);
    void setZRotation(int angle);
    void setXTranslate(double distance);
    void setYTranslate(double distance);
    void setZTranslate(double distance);
    void setViewZoomIn(double zoomScale);
    void setViewZoomOut(double zoomScale);

signals:
    // signaling rotation from mouse movement
    void xRotationChanged(int angle);
    void yRotationChanged(int angle);
    void zRotationChanged(int angle);

private:
    void draw();

    int xRot;
    int yRot;
    int zRot;
    double xTranslate;
    double yTranslate;
    double zTranslate;
    double xZoomScale;
    double yZoomScale;
    double zZoomScale;

    bool m_bShowAxis;
    bool m_bUseRgbComponent;

    QPoint lastPos;

    std::vector<ColorPoint3D_S> m_pointCloud;
};

#endif // ORBBEC_POINTCLOUD_GL_WIDGET_H
