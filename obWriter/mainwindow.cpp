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
    ir_gain = 0,
    ldp_en = 0,
    laser_en = 1,
    sn[32] = { 0 },
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
    ui->typeLabel->setText("Sensor type : " + QString::fromLocal8Bit((char*)type));

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
    ui->writeText->setStyleSheet("background-color: rgb(255, 255, 255)");
    ui->writeText->setText("");
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

void MainWindow::qString2CamParam(QString string, OBCameraParams* params)
{
    if ((string == NULL) || (string == "Write params err!")) {
        ui->writeText->setText("Write params err!");
        ui->writeText->setStyleSheet("background-color: rgb(255, 166, 166)");
    } else {
        QStringList mainList = string.split("\n");
        if (mainList.size() != 18) {
            ui->writeText->setText("Write params err!");
            ui->writeText->setStyleSheet("background-color: rgb(255, 166, 166)");
        } else {
            QStringList subList[10];

            // [IR Camera Intrinsic]
            subList[0] = mainList.at(1).split(" ");
            params->l_intr_p[0] = subList[0].at(0).toFloat();
            params->l_intr_p[2] = subList[0].at(2).toFloat();
            subList[1] = mainList.at(2).split(" ");
            params->l_intr_p[1] = subList[1].at(1).toFloat();
            params->l_intr_p[3] = subList[1].at(2).toFloat();

            // [RGB Camera Intrinsic]
            subList[2] = mainList.at(5).split(" ");
            params->r_intr_p[0] = subList[2].at(0).toFloat();
            params->r_intr_p[2] = subList[2].at(2).toFloat();
            subList[3] = mainList.at(6).split(" ");
            params->r_intr_p[1] = subList[3].at(1).toFloat();
            params->r_intr_p[3] = subList[3].at(2).toFloat();

            // [RGB to IR Camera Rotation]
            subList[4] = mainList.at(9).split(" ");
            params->r2l_r[0] = subList[4].at(0).toFloat();
            params->r2l_r[1] = subList[4].at(1).toFloat();
            params->r2l_r[2] = subList[4].at(2).toFloat();
            subList[5] = mainList.at(10).split(" ");
            params->r2l_r[3] = subList[5].at(0).toFloat();
            params->r2l_r[4] = subList[5].at(1).toFloat();
            params->r2l_r[5] = subList[5].at(2).toFloat();
            subList[6] = mainList.at(11).split(" ");
            params->r2l_r[6] = subList[6].at(0).toFloat();
            params->r2l_r[7] = subList[6].at(1).toFloat();
            params->r2l_r[8] = subList[6].at(2).toFloat();

            // [RGB to IR Camera Translation]
            subList[7] = mainList.at(13).split(" ");
            params->r2l_t[0] = subList[7].at(0).toFloat();
            params->r2l_t[1] = subList[7].at(1).toFloat();
            params->r2l_t[2] = subList[7].at(2).toFloat();

            // [IR Camera Distorted Params]
            subList[8] = mainList.at(15).split(" ");
            params->l_k[0] = subList[8].at(0).toFloat();
            params->l_k[1] = subList[8].at(1).toFloat();
            params->l_k[2] = subList[8].at(2).toFloat();
            params->l_k[3] = subList[8].at(3).toFloat();
            params->l_k[4] = subList[8].at(4).toFloat();

            // [RGB Camera Distorted Params]
            subList[9] = mainList.at(17).split(" ");
            params->r_k[0] = subList[9].at(0).toFloat();
            params->r_k[1] = subList[9].at(1).toFloat();
            params->r_k[2] = subList[9].at(2).toFloat();
            params->r_k[3] = subList[9].at(3).toFloat();
            params->r_k[4] = subList[9].at(4).toFloat();
        }
    }
}

void MainWindow::on_pushButton_clicked()
{
    resetTextEdit();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QString(),
                tr("All Files (*);;Text Files (*.txt);;Config Files (*.ini)"));

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
            return;
        }
        QTextStream in(&file);
        ui->lineEdit->setText(fileName);
        ui->writeText->setText(in.readAll());
        file.close();
    }
}

void MainWindow::on_writeButton_clicked()
{
    int size = 0;
    Status ret = STATUS_OK;
    QString str = ui->writeText->toPlainText();
    if ((str == NULL) || (str == "Write params err!")) {
        ui->writeText->setText("Write params err!");
        ui->writeText->setStyleSheet("background-color: rgb(255, 166, 166)");
        return;
    }

    OBCameraParams params = {0};
    qString2CamParam(str, &params);
    str = ui->writeText->toPlainText();
    if (str == "Write params err!") {
        ui->writeText->setText("Write params err!");
        ui->writeText->setStyleSheet("background-color: rgb(255, 166, 166)");
        return;
    }

    printf("-------------------------write-------------------------\n");
    printf("%f %f %f %f\n",
           params.l_intr_p[0],params.l_intr_p[1],params.l_intr_p[2],params.l_intr_p[3]);
    printf("%f %f %f %f\n",
           params.r_intr_p[0],params.r_intr_p[1],params.r_intr_p[2],params.r_intr_p[3]);
    printf("%f %f %f %f %f %f %f %f %f\n",
           params.r2l_r[0],params.r2l_r[1],params.r2l_r[2],params.r2l_r[3],
           params.r2l_r[4],params.r2l_r[5],params.r2l_r[6],params.r2l_r[7],params.r2l_r[8]);
    printf("%f %f %f\n",
           params.r2l_t[0],params.r2l_t[1],params.r2l_t[2]);
    printf("%f %f %f %f %f\n", params.l_k[0],params.l_k[1],params.l_k[2],params.l_k[3],params.l_k[4]);
    printf("%f %f %f %f %f\n", params.r_k[0],params.r_k[1],params.r_k[2],params.r_k[3],params.r_k[4]);
    printf("-------------------------------------------------------\n");


    size = sizeof(OBCameraParams);
    sensor.setProperty(openni::OBEXTENSION_ID_CAM_PARAMS, (uint8_t *)&params, size);
    if (ret != STATUS_OK) {
        cout << "Write params err : " << OpenNI::getExtendedError() << endl;
        ui->writeText->setText("Write params err!");
        ui->writeText->setStyleSheet("background-color: rgb(255, 166, 166)");
    } else {
        ui->writeText->setStyleSheet("background-color: rgb(166, 255, 166)");
    }
}

void MainWindow::on_readButton_clicked()
{
    int size = sizeof(OBCameraParams);
    sensor.getProperty(openni::OBEXTENSION_ID_CAM_PARAMS, (uint8_t*)&cam_params, &size);
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

    if (ui->readText->toPlainText() != ui->writeText->toPlainText()) {
        ui->readText->setStyleSheet("background-color: rgb(255, 166, 166)");
        ui->writeText->setStyleSheet("background-color: rgb(255, 166, 166)");
    } else {
        ui->readText->setStyleSheet("background-color: rgb(166, 255, 166)");
        ui->writeText->setStyleSheet("background-color: rgb(166, 255, 166)");
    }
}
