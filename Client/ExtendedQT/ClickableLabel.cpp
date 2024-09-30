#include "ClickableLabel.h"

ClickableLabel::ClickableLabel(QWidget* parent) : QLabel(parent)
{
    setMouseTracking(true);
}

void ClickableLabel::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit doubleClicked();
    }
}
