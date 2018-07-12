#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <OpenNI.h>

using namespace openni;

typedef struct OBCameraParams
{
    float l_intr_p[4];//[fx,fy,cx,cy]
    float r_intr_p[4];//[fx,fy,cx,cy]
    float r2l_r[9];//[r00,r01,r02;r10,r11,r12;r20,r21,r22]
    float r2l_t[3];//[t1,t2,t3]
    float l_k[5];//[k1,k2,p1,p2,k3]
    float r_k[5];//[k1,k2,p1,p2,k3]
    int is_mirror;
} OBCameraParams;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_writeButton_clicked();

    void on_readButton_clicked();

private:
    void initSensor();
    void deinitSensor();
    void resetTextEdit();
    QString camParam2QString(OBCameraParams* params);
    void qString2CamParam(QString string, OBCameraParams* params);

    Ui::MainWindow *ui;
    Device sensor;
    uint32_t ir_gain;
    uint32_t ldp_en;
    uint32_t laser_en;
    uint8_t sn[32];
    uint8_t type[32];
    OBCameraParams cam_params;

};

#endif // MAINWINDOW_H
