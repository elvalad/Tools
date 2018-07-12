#include <QOpenGLWidget>
#include <QtOpenGL>
#include <iostream>
#include <fstream>
#include <omp.h>

#include "orbbec_pointcloud_gl_widget.h"

using namespace std;

PointcloudGLWidget::PointcloudGLWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
    xRot = 0;
    yRot = 0;
    zRot = 0;
    xTranslate = 0.0;
    yTranslate = 0.0;
    zTranslate = -12.0;
    xZoomScale = 1.0;
    yZoomScale = 1.0;
    zZoomScale = 1.0;
}

PointcloudGLWidget::~PointcloudGLWidget()
{

}

void PointcloudGLWidget::initializeGL()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_FLAT);
    glClear(GL_COLOR_BUFFER_BIT);
}

void PointcloudGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glTranslatef(xTranslate, yTranslate, zTranslate);
    glScalef(xZoomScale, yZoomScale, zZoomScale);

    glRotatef(xRot / 16.0, 1.0, 0.0, 0.0);
    glRotatef(yRot / 16.0, 0.0, 1.0, 0.0);
    glRotatef(zRot / 16.0, 0.0, 0.0, 1.0);

    draw();
    update();
}

void PointcloudGLWidget::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-2, +2, -2, +2, 1.0, 15.0);
    glMatrixMode(GL_MODELVIEW);
}

void PointcloudGLWidget::updatePointcloud(std::vector<ColorPoint3D_S>& vPointCloud, bool bShowAxis, bool bUseRgbComponent)
{
    m_pointCloud = vPointCloud;
    m_bShowAxis = bShowAxis;
    m_bUseRgbComponent = bUseRgbComponent;
}

void PointcloudGLWidget::savePly(QString serialNum)
{
    QDateTime time = QDateTime::currentDateTime();
    QString str = time.toString("yyyy_MM_dd_hh_mm_ss");
    QString plyFileName = "./pointcloud_";
    ofstream ouF;

    if (m_bUseRgbComponent)
    {
        plyFileName.append(serialNum).append("_").append(str).append(".ply");
        ouF.open(plyFileName.toStdString(), ofstream::out);
        if (!ouF)
        {
            cerr << "failed to open the file." << endl;
            return;
        }

        ouF << "ply" << std::endl;
        ouF << "format ascii 1.0" << std::endl;
        ouF << "comment made by Orbbec " << std::endl;
        ouF << "comment Orbbec Co.,Ltd." << std::endl;
        ouF << "element vertex " << m_pointCloud.size() << std::endl;
        ouF << "property float32 x" << std::endl;
        ouF << "property float32 y" << std::endl;
        ouF << "property float32 z" << std::endl;
        ouF << "property uint8 red" << std::endl;
        ouF << "property uint8 green" << std::endl;
        ouF << "property uint8 blue" << std::endl;
        ouF << "element face 0" << std::endl;
        ouF << "property list uint8 int32 vertex_index" << std::endl;
        ouF << "end_header" << std::endl;

        for (unsigned int i = 0; i < m_pointCloud.size(); i++)
        {
            float x = m_pointCloud.at(i).x;
            float y = m_pointCloud.at(i).y;
            float z = m_pointCloud.at(i).z;
            float r = m_pointCloud.at(i).r;
            float g = m_pointCloud.at(i).g;
            float b = m_pointCloud.at(i).b;
            ouF << x << " " << y << " " << z << " " << r << " " << g << " " << b << endl;
        }

        ouF.close();
    }
    else
    {
        plyFileName.append(serialNum).append("_").append(str).append(".xyz");
        ouF.open(plyFileName.toStdString(), ofstream::out);
        if (!ouF)
        {
            cerr << "failed to open the file." << endl;
            return;
        }

        for (unsigned int i = 0; i < m_pointCloud.size(); i++)
        {
            float x = m_pointCloud.at(i).x;
            float y = m_pointCloud.at(i).y;
            float z = m_pointCloud.at(i).z;
            ouF << x << " " << y << " " << z << endl;
        }

        ouF.close();
    }
}

void PointcloudGLWidget::resetPointcloud()
{
    xRot = 0;
    yRot = 0;
    zRot = 0;
    xTranslate = 0.0;
    yTranslate = 0.0;
    zTranslate = -12.0;
    xZoomScale = 1.0;
    yZoomScale = 1.0;
    zZoomScale = 1.0;
}

void PointcloudGLWidget::draw()
{
	glPointSize(0.1f);
    glBegin(GL_POINTS);
    for (unsigned int i = 0; i < m_pointCloud.size(); i++)
    {
        if (m_bUseRgbComponent)
        {
            glColor3ub(m_pointCloud.at(i).r, m_pointCloud.at(i).g, m_pointCloud.at(i).b);
        }
        else
        {
            glColor3ub(128, 128, 128);
        }
        glVertex3f(m_pointCloud.at(i).x, m_pointCloud.at(i).y, m_pointCloud.at(i).z);
    }
    glEnd();

    if (m_bShowAxis)
    {
        glLineWidth(2.0);
        glBegin(GL_LINES);
        // draw line for x axis
        glColor3f(1.0, 0.0, 0.0);
        glVertex3f((GLfloat)0.0,  (GLfloat)0.0,  (GLfloat)0.0);
        glVertex3f((GLfloat)0.3,  (GLfloat)0.0,  (GLfloat)0.0);
        glVertex3f((GLfloat)0.26, (GLfloat)0.02, (GLfloat)0.0);
        glVertex3f((GLfloat)0.3,  (GLfloat)0.0,  (GLfloat)0.0);
        glVertex3f((GLfloat)0.26, (GLfloat)-0.02,(GLfloat)0.0);
        glVertex3f((GLfloat)0.3,  (GLfloat)0.0,  (GLfloat)0.0);
        // draw line for y axis
        glColor3f(0.0, 1.0, 0.0);
        glVertex3f((GLfloat)0.0,   (GLfloat)0.0,  (GLfloat)0.0);
        glVertex3f((GLfloat)0.0,   (GLfloat)0.3,  (GLfloat)0.0);
        glVertex3f((GLfloat)-0.02, (GLfloat)0.26, (GLfloat)0.0);
        glVertex3f((GLfloat)0.0,   (GLfloat)0.3,  (GLfloat)0.0);
        glVertex3f((GLfloat)0.02,  (GLfloat)0.26, (GLfloat)0.0);
        glVertex3f((GLfloat)0.0,   (GLfloat)0.3,  (GLfloat)0.0);
        // draw line for Z axis
        glColor3f(0.0, 0.0, 1.0);
        glVertex3f((GLfloat)0.0,   (GLfloat)0.0, (GLfloat)0.0);
        glVertex3f((GLfloat)0.0,   (GLfloat)0.0, (GLfloat)0.3);
        glVertex3f((GLfloat)0.02,  (GLfloat)0.0, (GLfloat)0.26);
        glVertex3f((GLfloat)0.0,   (GLfloat)0.0, (GLfloat)0.3);
        glVertex3f((GLfloat)-0.02, (GLfloat)0.0, (GLfloat)0.26);
        glVertex3f((GLfloat)0.0,   (GLfloat)0.0, (GLfloat)0.3);
        glEnd();
    }
}

void PointcloudGLWidget::qNormalizeAngle(int &angle)
{
    while (angle < 0)
        angle += 360 * 16;
    while (angle > 360)
        angle -= 360 * 16;
}

void PointcloudGLWidget::setXRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != xRot)
    {
        xRot = angle;
        emit xRotationChanged(angle);
    }
}

void PointcloudGLWidget::setYRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != yRot)
    {
        yRot = angle;
        emit yRotationChanged(angle);
    }
}

void PointcloudGLWidget::setZRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != zRot)
    {
        zRot = angle;
        emit zRotationChanged(angle);
    }
}

void PointcloudGLWidget::setXTranslate(double distance)
{
    xTranslate += distance / 1000.0;
}

void PointcloudGLWidget::setYTranslate(double distance)
{
   yTranslate -= distance / 1000.0;
}

void PointcloudGLWidget::setZTranslate(double distance)
{
   zTranslate += distance / 1000.0;
}

void PointcloudGLWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void PointcloudGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton)
    {
        setXRotation(xRot + 1 * dy);
        setYRotation(yRot + 1 * dx);
        setZRotation(zRot + 1 * dx);
    }
    else if (event->buttons() & Qt::RightButton)
    {
        setXTranslate(4 * dx);
        setYTranslate(4 * dy);
    }

    lastPos = event->pos();
}

void PointcloudGLWidget::setViewZoomIn(double zoomScale)
{
    xZoomScale += zoomScale;
    yZoomScale += zoomScale;
    zZoomScale += zoomScale;
}

void PointcloudGLWidget::setViewZoomOut(double zoomScale)
{
    if (xZoomScale > 0.1)
    {
        xZoomScale -= zoomScale;
    }

    if (yZoomScale > 0.1)
    {
        yZoomScale -= zoomScale;
    }

    if (zZoomScale > 0.1)
    {
        zZoomScale -= zoomScale;
    }
}

void PointcloudGLWidget::wheelEvent(QWheelEvent *event)
{
    if (event->delta() > 0)
    {
        emit setViewZoomIn(0.1);
    }
    else if (event->delta() < 0)
    {
        emit setViewZoomOut(0.1);
    }
}
