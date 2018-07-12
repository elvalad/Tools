#include <iostream>
#include <stdint.h>
#include <fstream>
#include <omp.h>
#include <QTimer>
#include <QtWidgets>
#include <QtConcurrent/qtconcurrentrun.h>

#include "orbbec_main_window.h"
#include "orbbec_common.h"
#include "orbbec_sensors.h"
#include "ui_orbbec_main_window.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_bOverlay(false),
    m_bShowAxis(true),
    m_bUseRgbComponent(true),
    m_bUseOwnParams(false),
    m_bUseExternalParams(false),
    m_croppingX(0),
    m_croppingY(0),
    sensors(new Sensors)
{
    parseConfigFile();

    ui->setupUi(this);

    //sensors->initCurSensor();
    sensors->getSensorList(sensorList);
    //sensors->startCurColor();
    //sensors->startCurDepth();

    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), SLOT(changeSensor(int)));
    connect(ui->overlayCheckBox, SIGNAL(stateChanged(int)), SLOT(toggleOverlay(int)));
    connect(ui->registrationCheckBox, SIGNAL(stateChanged(int)), SLOT(toggleImageRegistration()));
    connect(ui->colorMirrorCheckBox, SIGNAL(stateChanged(int)), SLOT(toggleColorMirror(int)));
    connect(ui->awbCheckBox, SIGNAL(stateChanged(int)), SLOT(toggleAutoWhiteBalance(int)));
    connect(ui->aeCheckBox, SIGNAL(stateChanged(int)), SLOT(toggleAutoExposure(int)));
    connect(ui->depthMirrorCheckBox, SIGNAL(stateChanged(int)), SLOT(toggleDepthMirror(int)));
    connect(ui->pointerModeCheckBox, SIGNAL(stateChanged(int)), SLOT(togglePointerMode(int)));
    connect(ui->showAxisCheckBox, SIGNAL(stateChanged(int)), SLOT(toggleShowAxis(int)));
    connect(ui->useRgbCheckBox, SIGNAL(stateChanged(int)), SLOT(toggleUseRgbComponent(int)));
    connect(ui->useOwnParamCheckBox, SIGNAL(stateChanged(int)), SLOT(toggleUseOwnParams(int)));
    connect(ui->useExternalParamCheckBox, SIGNAL(stateChanged(int)), SLOT(toggleUseExternalParams(int)));
    connect(ui->multiPointcloudButton, SIGNAL(released()), SLOT(showMultiPointcloud()));
    // hide multi pointcloud button
    ui->multiPointcloudButton->hide();
    connect(ui->savePlyButton, SIGNAL(released()), SLOT(savePly()));
    connect(ui->resetButton, SIGNAL(released()), SLOT(resetPointcloud()));
    showInfo();
    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(processFrame()));
    timer->start(30);
}

MainWindow::~MainWindow()
{
    delete sensors;
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape)
        close();
    else
        QWidget::keyPressEvent(e);
}

void MainWindow::processFrame()
{
    showColor();
    showDepth();
    showPointCloud();
}

void MainWindow::showInfo()
{
    for (int i = 0; i < sensorList.getSize(); i++)
    {
        QString str = QString("%1 %2%3").arg(sensorList[i].getName()).arg("#").arg(i);
        ui->comboBox->addItem(str);
    }
}

void MainWindow::showColor()
{
    QImage colorImage(sensors->getCurColorData(m_bOverlay), sensors->getCurColorReso().w, sensors->getCurColorReso().h, QImage::Format_RGB888);
    ui->colorLabel->setPixmap(QPixmap::fromImage(colorImage).scaled(sensors->getCurColorReso().w, sensors->getCurColorReso().h,
                                                                    Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::showDepth()
{
    QImage depthImage(sensors->getCurDepthData(), sensors->getCurDepthReso().w, sensors->getCurDepthReso().h, QImage::Format_ARGB32);
    ui->depthLabel->setPixmap(QPixmap::fromImage(depthImage).scaled(sensors->getCurDepthReso().w, sensors->getCurDepthReso().h,
                                                                    Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    ui->depthLabel->updateDepthValue(sensors->getCurDepthValue(), sensors->getCurDepthReso());
}

void MainWindow::showPointCloud()
{
    std::vector<ColorPoint3D_S> pointCloud;
    sensors->generatePointCloud(pointCloud, m_bUseOwnParams, m_bUseExternalParams, m_croppingX, m_croppingY);
    ui->pointCloudGLWidget->updatePointcloud(pointCloud, m_bShowAxis, m_bUseRgbComponent);
}

void MainWindow::changeSensor(int index)
{
    SensorInfo_S sInfo;
    sensors->stopCurColor();
    sensors->stopCurDepth();
    sensors->deinitCurSensor();
    sensors->initChangedSensor(sensorList[index].getUri());
    sensors->startCurColor();
    sensors->startCurDepth();
    sensors->getCurSensorInfo(sInfo);
    ui->serialLabel->setText(sInfo.serialNum);
    ui->sensorNameLabel->setText(sensorList[index].getName());
}

void MainWindow::showMultiPointcloud()
{
    std::cout << "test multi pointcloud" << std::endl;
}

void MainWindow::savePly()
{
    SensorInfo_S sInfo;
    sensors->getCurSensorInfo(sInfo);
    QtConcurrent::run(this->ui->pointCloudGLWidget, &PointcloudGLWidget::savePly, QString(sInfo.serialNum));
}

void MainWindow::resetPointcloud()
{
    ui->pointCloudGLWidget->resetPointcloud();
}

void MainWindow::togglePointerMode(int state)
{
    ui->depthLabel->setPointMode(state);
}

void MainWindow::toggleAutoWhiteBalance(int state)
{
    sensors->toggleAutoWhiteBalance(state);
}

void MainWindow::toggleAutoExposure(int state)
{
    sensors->toggleAutoExposure(state);
}

void MainWindow::toggleColorMirror(int state)
{
    sensors->toggleColorMirror(state);
}

void MainWindow::toggleDepthMirror(int state)
{
    sensors->toggleDepthMirror(state);
}

void MainWindow::toggleImageRegistration()
{
    sensors->toggleImageRegistration();
}

void MainWindow::toggleOverlay(int state)
{
    if (state == Qt::Checked)
    {
        ui->depthLabel->hide();
        m_bOverlay = true;
    }
    else if (state == Qt::Unchecked)
    {
        ui->depthLabel->show();
        m_bOverlay = false;
    }
}

void MainWindow::toggleShowAxis(int state)
{
    if (state == Qt::Checked)
    {
        m_bShowAxis = true;
    }
    else if (state == Qt::Unchecked)
    {
        m_bShowAxis = false;
    }
}

void MainWindow::toggleUseRgbComponent(int state)
{
    if (state == Qt::Checked)
    {
        m_bUseRgbComponent = true;
    }
    else if (state == Qt::Unchecked)
    {
        m_bUseRgbComponent = false;
    }
}

void MainWindow::toggleUseOwnParams(int state)
{
    if (state == Qt::Checked)
    {
        m_bUseOwnParams = true;
    }
    else if (state == Qt::Unchecked)
    {
        m_bUseOwnParams = false;
    }
}

void MainWindow::toggleUseExternalParams(int state)
{
    if (state == Qt::Checked)
    {
        m_bUseExternalParams = true;
    }
    else if (state == Qt::Unchecked)
    {
        m_bUseExternalParams = false;
    }
}

void MainWindow::parseConfigFile()
{
    QString configFileName = "./astra_portal.ini";
    QFileInfo fileInfo(configFileName);
    if (fileInfo.isFile())
    {
        cout << "Config file name : " << configFileName.toStdString() << endl;
        char tmpbuf[128];
        int tmp;
        std::ifstream inf;
        inf.open(configFileName.toStdString(), std::ios::in);
        inf.getline(tmpbuf, sizeof(tmpbuf));
        inf >> tmp; m_croppingX = tmp;
        inf >> tmp; m_croppingY = tmp;
        inf.close();
    }
    else
    {
        cout << "Config file not exist." << endl;
        m_croppingX = 0;
        m_croppingY = 0;
    }
}

