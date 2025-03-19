#include "timerbtn.h"
#include <QMouseEvent>


TimerBtn::TimerBtn(QWidget *parent) : QPushButton(parent), _counter(10)
{
    this->_timer = new QTimer(this);
    // 连接信号
    connect(_timer, &QTimer::timeout, [this]() {
        this->_counter--;
        // 倒计时为0，则重置状态并使能按钮
        if (this->_counter <= 0) {
            this->_timer->stop();
            this->_counter = 10;
            TimerBtn::_email_ready = false;
            this->setText("获取");
            this->setEnabled(true);
            return;
        }
        // 倒计时不为0，则显示剩余秒数
        this->setText(QString::number(this->_counter));
    });
}

TimerBtn::~TimerBtn()
{
    this->_timer->stop();
}

void TimerBtn::mouseReleaseEvent(QMouseEvent *e)
{
    // 调用基类的mouseReleaseEvent，确保其他默认行为得以保留
    QPushButton::mouseReleaseEvent(e);

    // 这里处理鼠标左键释放事件
    if (_email_ready && e->button() == Qt::LeftButton) {
        qDebug() << "MyButton was released!";
        this->setEnabled(false);
        this->setText(QString::number(_counter));
        // 启动 timer，每1000ms timeout一次
        this->_timer->start(1000);
        emit click();
    }
}
