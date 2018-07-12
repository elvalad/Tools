#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>
#include <vector>
#include <QTextCodec>

using namespace std;
using namespace openni;

OBExtensionCommand  OBExtensionList[] =  //
{
    { OBEXTENSION_ID_IR_GAIN, EXTENSION_INFO_GET | EXTENSION_INFO_SET, 4 },
    { OBEXTENSION_ID_LDP_EN, EXTENSION_INFO_GET | EXTENSION_INFO_SET, 4 },
    { OBEXTENSION_ID_CAM_PARAMS, EXTENSION_INFO_GET | EXTENSION_INFO_SET, sizeof(OBCameraParams) },
    { OBEXTENSION_ID_LASER_EN, EXTENSION_INFO_SET, 4 },
    { OBEXTENSION_ID_SERIALNUMBER,  EXTENSION_INFO_GET, 12 },
    { OBEXTENSION_ID_DEVICETYPE, EXTENSION_INFO_GET, 32 }
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ir_gain = 0;
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
    Status ret = STATUS_OK;
    ret = OpenNI::initialize();
    cout << "OpenNI initialization : " << OpenNI::getExtendedError() << endl;

    const char* deviceURI = ANY_DEVICE;
    ret = sensor.open(deviceURI);
    if (ret != 0 )
    {
        cout << "Sensor open : " << OpenNI::getExtendedError() << endl;
        ui->textEdit->setText("摄像头打开失败，请检查连接！");
        ui->textEdit->setStyleSheet("background-color: rgb(255, 166, 166)");
        ui->pushButton->setEnabled(false);
        return;
    }

    ret = OBExtension_Init(&ext, &sensor);
    cout << "Orbbec extension initialization : " << OpenNI::getExtendedError() << endl;

    OBExtension_GetProperty(&ext, OBEXTENSION_ID_DEVICETYPE, type, OBExtensionList[OBEXTENSION_ID_DEVICETYPE].datasize);
    cout << "Sensor type : " << type << endl;
    ui->typeLabel->setText("摄像头类型：" + (QString::fromLocal8Bit((char*)type)));

    OBExtension_GetProperty(&ext, OBEXTENSION_ID_SERIALNUMBER, sn, OBExtensionList[OBEXTENSION_ID_SERIALNUMBER].datasize);
    cout << "Sensor serial number : " << sn << endl;
    ui->snLabel->setText("摄像头序列号: " + QString::fromLocal8Bit((char*)sn));

    date = QString::fromLocal8Bit((char*)sn);
    ui->dateLabel->setText("生产日期: " + date.mid(0, 2) + "-" + date.mid(2, 2) + "-" + date.mid(4, 2));

    OBExtension_GetProperty(&ext, OBEXTENSION_ID_IR_GAIN, (uint8_t*)&ir_gain, OBExtensionList[OBEXTENSION_ID_IR_GAIN].datasize);
    cout << "Sensor gain value : " << ir_gain << endl;
    ui->gainLabel->setText("摄像头增益值：0x" + QString::number(ir_gain, 16));
}

void MainWindow::deinitSensor()
{
    if (sensor.isValid())
    {
        sensor.close();
        cout << "Sensor close : " << OpenNI::getExtendedError() << endl;
    }

    OpenNI::shutdown();
    cout << "OpenNI shutdown" << endl;
}

void MainWindow::on_pushButton_clicked()
{
    QString str = ui->textEdit->toPlainText();
    bool isNumber;

    if (str == NULL)
    {
        ui->textEdit->setText("增益值不能为空！");
        ui->textEdit->setStyleSheet("background-color: rgb(255, 166, 166)");
        return;
    }

    ir_gain = str.toInt(&isNumber, 16);
    if (!isNumber)
    {
        ui->textEdit->setText("请输入正确的增益值！");
        ui->textEdit->setStyleSheet("background-color: rgb(255, 166, 166)");
        return;
    }
    else
    {
        OBExtension_SetProperty(&ext, OBEXTENSION_ID_IR_GAIN, (uint8_t*)&ir_gain, OBExtensionList[OBEXTENSION_ID_IR_GAIN].datasize);
        ui->textEdit->setText("当前摄像头更新成功，请退出程序！");
        ui->textEdit->setStyleSheet("background-color: rgb(166, 255, 166)");
        return;
    }

    /*Array<DeviceInfo> sensorList;
    OpenNI::enumerateDevices(&sensorList);
    cout << "sensor size : " << sensorList.getSize() << endl;
    if (sensorList.getSize() > 1)
    {
        ui->textEdit->setText("一次只能更新一个摄像头!");
        ui->textEdit->setStyleSheet("background-color: rgb(255, 166, 166)");
        ui->pushButton->setEnabled(false);
        return;
    }

    if (QString::fromLocal8Bit((char*)type) != "Orbbec Astra S")
    {
        ui->textEdit->setText("当前摄像头不需要更新！");
        ui->textEdit->setStyleSheet("background-color: rgb(255, 166, 166)");
        ui->pushButton->setEnabled(false);
        return;
    }

    QStringList dateList;
    //dateList << "17-05-01" << "17-05-02" << "17-05-03" << "17-05-04" << "17-05-05" << "17-05-06" << "17-05-07" << "17-05-08";
    dateList << "17-03-01" << "17-03-02" << "17-03-03" << "17-03-04" << "17-03-05" << "17-03-06" << "17-03-07" << "17-03-08";
    QString formatDate = date.mid(0, 2) + "-" + date.mid(2, 2) + "-" + date.mid(4, 2);
    if (!dateList.contains(formatDate))
    {
        ui->textEdit->setText("当前摄像头不需要更新！");
        ui->textEdit->setStyleSheet("background-color: rgb(255, 166, 166)");
        ui->pushButton->setEnabled(false);
        return;
    }

    if (ir_gain == 0x60)
    {
        ui->textEdit->setText("当前摄像头已经更新，请退出程序！");
        ui->textEdit->setStyleSheet("background-color: rgb(166, 255, 166)");
    }
    else
    {
        ir_gain = 0x60;
        OBExtension_SetProperty(&ext, OBEXTENSION_ID_IR_GAIN, (uint8_t*)&ir_gain, OBExtensionList[OBEXTENSION_ID_IR_GAIN].datasize);
        ui->textEdit->setText("当前摄像头更新成功，请退出程序！");
        ui->textEdit->setStyleSheet("background-color: rgb(166, 255, 166)");
    }*/
}
