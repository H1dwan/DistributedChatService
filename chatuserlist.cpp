#include "chatuserlist.h"
#include <QEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include "usermgr.h"

ChatUserList::ChatUserList(QWidget *parent) : QListWidget(parent)
{
    // 禁用滚动条（方便自定义滚动行为）
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 安装事件过滤器（需要重写 eventFilter函数来处理事件）
    this->viewport()->installEventFilter(this);
}

bool ChatUserList::eventFilter(QObject *watched, QEvent *event)
{
    // 处理视口 viewport的鼠标悬浮进入或离开事件
    if (watched == this->viewport())
    {
        if (event->type() == QEvent::Enter)         // 鼠标进入视口，显示滚动条
        {
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        }
        else if (event->type() == QEvent::Leave)    // 鼠标离开视口，隐藏滚动条
        {
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
    }

    // 处理视口 viewport的鼠标滚轮事件
    if (watched == this->viewport() && event->type() == QEvent::Wheel)
    {
        QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
        int numDegrees = wheelEvent->angleDelta().y() / 8;
        int numSteps = numDegrees / 15;  // 计算滚动步数

        // 更新垂直滚动条的值，通过减少步数来向上滚动内容
        this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() - numSteps);
        // 检查是否滚动到底部
        QScrollBar* scrollBar = this->verticalScrollBar();
        int maxScrollValue = scrollBar->maximum();
        int currentValue = scrollBar->value();

        if (maxScrollValue - currentValue <= 0)
        {
            // 如果所有聊天条目都已加载完毕，则返回
            if (UserMgr::GetInstance()->IsLoadChatFin())
            {
                return true;
            }
            // 滚动到底部，加载新的联系人
            qDebug()<<"loading more chat user...";
            // 发送信号通知聊天界面加载更多聊天内容
            // TODO: 更到最新
            emit sig_loading_chat_users();
        }
        return true;    // 停止事件传递
    }
    return QListWidget::eventFilter(watched, event);
}
