#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <OBExtension.h>
#include <OpenNI.h>

using namespace openni;

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

private:
    void initSensor();
    void deinitSensor();

    Ui::MainWindow *ui;

    Device sensor;
    OBExtension ext;
    uint32_t ir_gain;
    uint8_t sn[32];
    uint8_t type[32];
    QString date;
};

#endif // MAINWINDOW_H
