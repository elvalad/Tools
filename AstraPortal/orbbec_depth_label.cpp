#include "orbbec_depth_label.h"

DepthLabel::DepthLabel(QWidget* parent)
{
    curPos.x = 0;
    curPos.y = 0;
    curDepth = 0;
    pointModeState = Qt::Unchecked;
    setMouseTracking(true);
}

DepthLabel::~DepthLabel()
{

}

void DepthLabel::updateDepthValue(const DepthPixel* pDepth, Resolution_S depthReso)
{
    curDepth = pDepth[curPos.y * depthReso.w + curPos.x];
    if (pointModeState == Qt::Checked)
    {
        QString str = QString("(%1, %2, %3)").arg(curPos.x).arg(curPos.y).arg(curDepth);
        QToolTip::showText(mouseEventPos, str);
    }
    else if (pointModeState == Qt::Unchecked)
    {
        QToolTip::hideText();
    }
}

Point2D_S DepthLabel::getPointPos()
{
    return curPos;
}

void DepthLabel::setPointDepth(uint16_t depth)
{
    curDepth = depth;
}

uint16_t DepthLabel::getPointDepth()
{
    return curDepth;
}

void DepthLabel::setPointMode(int state)
{
    pointModeState = state;
}

int DepthLabel::getPointMode()
{
    return pointModeState;
}

void DepthLabel::mouseMoveEvent(QMouseEvent *event)
{
    //std::cout << "mouse move event " << event->x() << " " << event->y() << std::endl;
    curPos.x = event->x();
    curPos.y = event->y();
    mouseEventPos = event->globalPos();
}

void DepthLabel::mousePressEvent(QMouseEvent *event)
{
    //std::cout << "mouse press event " << event->x() << " " << event->y() << std::endl;
}

void DepthLabel::mouseReleaseEvent(QMouseEvent *event)
{
    //std::cout << "mouse release event " << event->x() << " " << event->y() << std::endl;
}
