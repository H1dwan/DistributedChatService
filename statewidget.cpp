#include "statewidget.h"
#include <QVBoxLayout>
#include <QStyleOption>
#include <QPainter>
#include <QMouseEvent>

StateWidget::StateWidget(QWidget *parent)
    : QWidget{parent}
{
    setCursor(Qt::PointingHandCursor);
    AddRedPoint();  // 添加红点
}

// 初始化各个状态的初始值
void StateWidget::SetState(QString normal, QString hover, QString press, QString select,
                           QString select_hover, QString select_press)
{
    _normal = normal;
    _normal_hover = hover;
    _normal_press = press;
    _selected = select;
    _selected_hover = select_hover;
    _selected_press = select_press;

    setProperty("state", "normal");
    repolish(this);
}

void StateWidget::SetSelected(bool be_selected)
{
    if (be_selected)
    {
        _cur_state = ClickLbState::Selected;
        setProperty("state", _selected);
        repolish(this);
        return;
    }
    _cur_state = ClickLbState::Normal;
    setProperty("state", _normal);
    repolish(this);
}

ClickLbState StateWidget::GetCurState()
{
    return _cur_state;
}

void StateWidget::ClearState()
{
    _cur_state = ClickLbState::Normal;
    setProperty("state", _normal);
    repolish(this);
}

/**
 * @brief StateWidget::AddRedPoint  在 StateWidget 中添加一个红色的点 _red_point（QLabel）
 */
void StateWidget::AddRedPoint()
{
    _red_point = new QLabel();
    _red_point->setObjectName("red_point");
    // 用于管理 StateWidget 中的子控件布局
    auto* layout = new QVBoxLayout();
    _red_point->setAlignment(Qt::AlignCenter);
    layout->addWidget(_red_point);
    layout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(layout);
    _red_point->setVisible(false);
}

void StateWidget::ShowRedPoint(bool show)
{
    _red_point->setVisible(show);
}

void StateWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

// 处理鼠标悬停进入事件
void StateWidget::enterEvent(QEnterEvent *event)
{
    if (_cur_state == ClickLbState::Normal)
    {
        setProperty("state", _normal_hover);
        repolish(this);
    }
    else
    {
        qDebug()<<"enter, change to selected hover: "<< _selected_hover;
        setProperty("state", _selected_hover);
        repolish(this);
    }
    QWidget::enterEvent(event);
}

// 处理鼠标悬停离开事件
void StateWidget::leaveEvent(QEvent *event)
{
    if (_cur_state == ClickLbState::Normal)
    {
        setProperty("state", _normal);
        repolish(this);
    }
    else
    {
        qDebug()<<"leave, change to select normal : "<< _selected;
        setProperty("state", _selected);
        repolish(this);
    }
    QWidget::leaveEvent(event);
}

// 处理鼠标点击事件
void StateWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (_cur_state == ClickLbState::Selected)
        {
            qDebug()<<"PressEvent, already to selected press: "<< _selected_press;
            QWidget::mousePressEvent(event);
            return;
        }
        else
        {
            qDebug()<<"PressEvent , change to selected press: "<< _selected_press;
            _cur_state = ClickLbState::Selected;
            setProperty("state",_selected_press);
            repolish(this);
        }
    }
}

// 处理鼠标释放事件
void StateWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if(_cur_state == ClickLbState::Normal)
        {
            //qDebug()<<"ReleaseEvent , change to normal hover: "<< _normal_hover;
            setProperty("state", _normal_hover);
            repolish(this);
        }
        else
        {
            //qDebug()<<"ReleaseEvent , change to select hover: "<< _selected_hover;
            setProperty("state", _selected_hover);
            repolish(this);
        }
        emit clicked();
        return;
    }
    // 调用基类的mousePressEvent以保证正常的事件处理
    QWidget::mousePressEvent(event);
}
