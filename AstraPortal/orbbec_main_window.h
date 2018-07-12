#ifndef ORBBEC_MAIN_WINDOW_H
#define ORBBEC_MAIN_WINDOW_H

#include <QMainWindow>

#include "orbbec_sensors.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event);

public slots:
    void processFrame();
    void changeSensor(int index);
    void showMultiPointcloud();
    void savePly();
    void resetPointcloud();
    void togglePointerMode(int state);
    void toggleAutoWhiteBalance(int state);
    void toggleAutoExposure(int state);
    void toggleColorMirror(int state);
    void toggleDepthMirror(int state);
    void toggleImageRegistration();
    void toggleOverlay(int state);
    void toggleShowAxis(int state);
    void toggleUseRgbComponent(int state);
    void toggleUseOwnParams(int state);
    void toggleUseExternalParams(int state);

private:
    void showInfo();
    void showColor();
    void showDepth();
    void showPointCloud();
    void parseConfigFile();

    Ui::MainWindow *ui;
    Sensors* sensors;
    Array<DeviceInfo> sensorList;
    bool m_bOverlay;
    bool m_bShowAxis;
    bool m_bUseRgbComponent;
    bool m_bUseOwnParams;
    bool m_bUseExternalParams;

    int m_croppingX;
    int m_croppingY;
};

#endif // ORBBEC_MAIN_WINDOW_H
