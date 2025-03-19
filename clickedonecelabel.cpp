#include "clickedonecelabel.h"
#include <QMouseEvent>

ClickedOneceLabel::ClickedOneceLabel(QWidget *parent)
    :QLabel(parent)
{
    setCursor(Qt::PointingHandCursor);
}

void ClickedOneceLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton)
    {
        emit clicked(this->text());
        return;
    }

    QLabel::mouseReleaseEvent(ev);
}


