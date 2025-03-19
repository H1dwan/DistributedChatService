#include "clickedlabel.h"
#include <QMouseEvent>

ClickedLabel::ClickedLabel(QWidget *parent) : QLabel(parent), _curstate(ClickLbState::Normal)
{
    // 设置鼠标为手形
    this->setCursor(Qt::PointingHandCursor);
}

/**
 * @brief ClickedLabel::mousePressEvent 处理鼠标左键点击事件，切换可见和不可见的图标
 * @param event
 */
void ClickedLabel::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        if (this->_curstate == ClickLbState::Normal) {
            qDebug() << "clicked, change to selected press." << _selected_press;
            _curstate = ClickLbState::Selected;
            setProperty("state", _selected_press);
            repolish(this);
        } else {
            qDebug() << "clicked, change to normal press." << _normal_press;
            _curstate = ClickLbState::Normal;
            setProperty("state", _normal_press);
            repolish(this);
        }
        return;
    }
    // 调用基类的mousePressEvent以保证正常的事件处理
    QLabel::mousePressEvent(event);
}

/**
 * @brief ClickedLabel::mouseReleaseEvent 处理鼠标左键释放事件
 * @param event
 */
void ClickedLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (this->_curstate == ClickLbState::Selected)
        {
            qDebug() << "ReleaseEvent , change to normal hover:." << _selected_hover;
            // _curstate = ClickLbState::Normal;
            setProperty("state",_selected_hover);
            repolish(this);
        }
        else
        {
            qDebug()<< "ReleaseEvent , change to normal hover: "<< _normal_hover;
            // _curstate = ClickLbState::Normal;
            setProperty("state",_normal_hover);
            repolish(this);
        }
        emit clicked(this->text(), _curstate);
        return;
    }
    QLabel::mouseReleaseEvent(event);
}

/**
 * @brief ClickedLabel::enterEvent   处理鼠标悬停进入事件
 * @param event
 */
void ClickedLabel::enterEvent(QEnterEvent *event)
{
     // 在这里处理鼠标悬停进入的逻辑
    if (_curstate == ClickLbState::Normal) {
        qDebug()<<"enter , change to normal hover: "<< _normal_hover;
        setProperty("state", _normal_hover);
        repolish(this);
    } else {
        qDebug()<<"enter , change to selected hover: "<< _selected_hover;
        setProperty("state", _selected_hover);
        repolish(this);
    }
    // 调用基类的方法以保证正常的事件处理
    QLabel::enterEvent(event);
}

/**
 * @brief ClickedLabel::leaveEvent  处理鼠标悬停离开事件
 * @param event
 */
void ClickedLabel::leaveEvent(QEvent *event)
{
    // 在这里处理鼠标悬停离开的逻辑
    if(_curstate == ClickLbState::Normal){
        qDebug()<<"leave , change to normal : "<< _normal;
        setProperty("state",_normal);
        repolish(this);
        update();

    }else{
        qDebug()<<"leave , change to normal hover: "<< _selected;
        setProperty("state",_selected);
        repolish(this);
        update();
    }
    QLabel::leaveEvent(event);
}

void ClickedLabel::SetState(QString normal, QString hover, QString press,
                            QString select, QString select_hover, QString select_press)
{
    _normal = normal;
    _normal_hover = hover;
    _normal_press = press;

    _selected = select;
    _selected_hover = select_hover;
    _selected_press = select_press;

    setProperty("state",normal);
    repolish(this);
}

bool ClickedLabel::SetCurState(ClickLbState state)
{
    _curstate = state;
    if (_curstate == ClickLbState::Normal) {
        setProperty("state", _normal);
        repolish(this);
    }
    else if (_curstate == ClickLbState::Selected) {
        setProperty("state", _selected);
        repolish(this);
    }

    return true;
}

ClickLbState ClickedLabel::GetCurState()
{
    return this->_curstate;
}

void ClickedLabel::ResetNormalState()
{
    _curstate = ClickLbState::Normal;
    setProperty("state", _normal);
    repolish(this);
}
