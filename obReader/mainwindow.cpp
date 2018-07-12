#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>

#include <iostream>

using namespace std;
using namespace openni;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ir_gain = 0;
    ldp_en = 0;
    laser_en = 1;
    sn[32] = { 0 };
    ui->setupUi(this);
    initSensor();
    Qt::WindowFlags flags = 0;
    flags |= Qt::WindowMinimizeButtonHint;
    flags |= Qt::WindowCloseButtonHint;
    setWindowFlags(flags);
}

MainWindow::~MainWindow()
{
    deinitSensor();
    delete ui;
}

void MainWindow::initSensor()
{
    int size = 0;
    Status ret = STATUS_OK;
    ret = OpenNI::initialize();
    cout << "OpenNI initialization : " << OpenNI::getExtendedError() << endl;

    const char* deviceURI = ANY_DEVICE;
    ret = sensor.open(deviceURI);
    cout << "Sensor open : " << OpenNI::getExtendedError() << endl;

    size = 32;
    sensor.getProperty(openni::OBEXTENSION_ID_DEVICETYPE, type, &size);
    cout << "Sensor type : " << type << endl;

    size = 12;
    sensor.getProperty(openni::OBEXTENSION_ID_SERIALNUMBER, sn, &size);
    cout << "Sensor serial number : " << sn << endl;
    ui->snLabel->setText("Sensor serial number : " + QString::fromLocal8Bit((char*)sn));
}

void MainWindow::deinitSensor()
{
    if (sensor.isValid()) {
        sensor.close();
        cout << "Sensor close : " << OpenNI::getExtendedError() << endl;
    }

    OpenNI::shutdown();
    cout << "OpenNI shutdown" << endl;
}

void MainWindow::resetTextEdit()
{
    ui->readText->setStyleSheet("background-color: rgb(255, 255, 255)");
    ui->readText->setText("");
}

QString MainWindow::camParam2QString(OBCameraParams* params)
{
    QString str = QString("[IR Camera Intrinsic]\n"
                          "%1 0 %2\n"
                          "0 %3 %4\n"
                          "0 0 1\n"
                          "[RGB Camera Intrinsic]\n"
                          "%5 0 %6\n"
                          "0 %7 %8\n"
                          "0 0 1\n"
                          "[RGB to IR Camera Rotation]\n"
                          "%9 %10 %11\n"
                          "%12 %13 %14\n"
                          "%15 %16 %17\n"
                          "[RGB to IR Camera Translation]\n"
                          "%18 %19 %20\n"
                          "[IR Camera Distorted Params]\n"
                          "%21 %22 %23 %24 %25\n"
                          "[RGB Camera Distorted Params]\n"
                          "%26 %27 %28 %29 %30").
            arg(params->l_intr_p[0]).arg(params->l_intr_p[2]).
            arg(params->l_intr_p[1]).arg(params->l_intr_p[3]).
            arg(params->r_intr_p[0]).arg(params->r_intr_p[2]).
            arg(params->r_intr_p[1]).arg(params->r_intr_p[3]).
            arg(params->r2l_r[0]).arg(params->r2l_r[1]).arg(params->r2l_r[2]).
            arg(params->r2l_r[3]).arg(params->r2l_r[4]).arg(params->r2l_r[5]).
            arg(params->r2l_r[6]).arg(params->r2l_r[7]).arg(params->r2l_r[8]).
            arg(params->r2l_t[0]).arg(params->r2l_t[1]).arg(params->r2l_t[2]).
            arg(params->l_k[0]).arg(params->l_k[1]).arg(params->l_k[2]).arg(params->l_k[3]).arg(params->l_k[4]).
            arg(params->r_k[0]).arg(params->r_k[1]).arg(params->r_k[2]).arg(params->r_k[3]).arg(params->r_k[4]);
    return str;
}

void MainWindow::on_readButton_clicked()
{
    int size = sizeof(OBCameraParams);
    sensor.getProperty(openni::OBEXTENSION_ID_CAM_PARAMS, (uint8_t*)&cam_params, &size);
    resetTextEdit();
    printf("-------------------------read-------------------------\n");
    printf("%f %f %f %f\n",
           cam_params.l_intr_p[0],cam_params.l_intr_p[1],cam_params.l_intr_p[2],cam_params.l_intr_p[3]);
    printf("%f %f %f %f\n",
           cam_params.r_intr_p[0],cam_params.r_intr_p[1],cam_params.r_intr_p[2],cam_params.r_intr_p[3]);
    printf("%f %f %f %f %f %f %f %f %f\n",
           cam_params.r2l_r[0],cam_params.r2l_r[1],cam_params.r2l_r[2],cam_params.r2l_r[3],
           cam_params.r2l_r[4],cam_params.r2l_r[5],cam_params.r2l_r[6],cam_params.r2l_r[7],cam_params.r2l_r[8]);
    printf("%f %f %f\n",
           cam_params.r2l_t[0],cam_params.r2l_t[1],cam_params.r2l_t[2]);
    printf("%f %f %f %f %f\n", cam_params.l_k[0],cam_params.l_k[1],cam_params.l_k[2],cam_params.l_k[3],cam_params.l_k[4]);
    printf("%f %f %f %f %f\n", cam_params.r_k[0],cam_params.r_k[1],cam_params.r_k[2],cam_params.r_k[3],cam_params.r_k[4]);
    printf("------------------------------------------------------\n");

    QString str = camParam2QString(&cam_params);

    if (str == NULL) {
        ui->readText->setText("Read params err!");
        ui->readText->setStyleSheet("background-color: rgb(255, 166, 166)");
        return;
    }
    ui->readText->setText(str);
}

void MainWindow::on_saveButton_clicked()
{
    if (ui->readText->toPlainText() == NULL || ui->readText->toPlainText() == "Save params err!") {
        ui->readText->setText("Save params err!");
        ui->readText->setStyleSheet("background-color: rgb(255, 166, 166)");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), QString(QString::fromLocal8Bit((char*)sn).append("_camera_params")),
                tr("Config Files (*.ini);;Text Files (*.txt)"));
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            ui->readText->setText("Save params err!");
            ui->readText->setStyleSheet("background-color: rgb(255, 166, 166)");
        } else {
            QTextStream stream(&file);
            stream << ui->readText->toPlainText();
            stream.flush();
            file.close();
            ui->readText->setStyleSheet("background-color: rgb(166, 255, 166)");
        }
    }
}
