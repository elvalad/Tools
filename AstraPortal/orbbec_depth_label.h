#ifndef ORBBEC_DEPTH_LABEL_H
#define ORBBEC_DEPTH_LABEL_H

#include <iostream>
#include <QtWidgets>
#include <QLabel>
#include <QMouseEvent>
#include <QToolTip>

#include "orbbec_common.h"

class DepthLabel : public QLabel
{
    Q_OBJECT

public:
    explicit DepthLabel(QWidget *parent = 0);
    ~DepthLabel();
    void updateDepthValue(const DepthPixel* pDepth, Resolution_S depthReso);
    Point2D_S getPointPos();
    void setPointDepth(uint16_t depth);
    uint16_t getPointDepth();
    void setPointMode(int state);
    int getPointMode();

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    Point2D_S curPos;
    uint16_t curDepth;
    QPoint mouseEventPos;
    int pointModeState;

};

#endif // ORBBEC_DEPTH_LABEL_H
