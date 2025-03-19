#include "clickedbtn.h"
#include "global.h"

ClickedBtn::ClickedBtn(QWidget *parent) : QPushButton(parent)
{
    // 将光标设置为手形
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::NoFocus);
}

ClickedBtn::~ClickedBtn()
{

}

void ClickedBtn::SetState(QString normal, QString hover, QString press)
{
    _normal = normal;
    _hover = hover;
    _press = press;
    setProperty("state", _normal);
    repolish(this);
}

void ClickedBtn::enterEvent(QEnterEvent *event)
{
    setProperty("state", _hover);
    repolish(this);
    QPushButton::enterEvent(event);
}

void ClickedBtn::leaveEvent(QEvent *event)
{
    setProperty("state", _normal);
    repolish(this);
    QPushButton::leaveEvent(event);
}

void ClickedBtn::mousePressEvent(QMouseEvent *e)
{
    setProperty("state", _press);
    repolish(this);
    QPushButton::mousePressEvent(e);
}

void ClickedBtn::mouseReleaseEvent(QMouseEvent *e)
{
    setProperty("state", _hover);
    repolish(this);
    QPushButton::mouseReleaseEvent(e);
}
