#include "chatview.h"
#include <QScrollBar>
#include <QTimer>
#include <QStyleOption>
#include <QPainter>
#include <QEvent>

ChatView::ChatView(QWidget* parent)
    : QWidget(parent), m_bIsAppending(false)
{
    QVBoxLayout* pMainLayout = new QVBoxLayout();
    pMainLayout->setContentsMargins(QMargins(0, 0, 0, 0));
    this->setLayout(pMainLayout);   // 将这个布局设置为当前窗口的主布局

    // 创建滚动区域
    m_pScrollArea = new QScrollArea();
    m_pScrollArea->setObjectName("chat_area");
    // 创建一个 QWidget 作为滚动区域的内容容器
    QWidget* w = new QWidget(this);
    w->setObjectName("chat_bg");
    w->setAutoFillBackground(true); // 设置自动填充背景
    // 创建一个 QVBoxLayout 布局，用于在内容容器中布置控件
    QVBoxLayout* pVLayout = new QVBoxLayout();
    pVLayout->addWidget(new QWidget(), 100000); // 在 w 的子布局下创建了一个 widget（相当于弹簧的作用）
    w->setLayout(pVLayout);
    // 将内容容器设置为滚动区域的子控件
    m_pScrollArea->setWidget(w);
    // 将滚动区域添加到主布局中
    pMainLayout->addWidget(m_pScrollArea);

    // 滚动条设置
    m_pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QScrollBar *pVScrollBar = m_pScrollArea->verticalScrollBar();
    // 连接垂直滚动条的槽函数，用于处理滚动条范围变化
    connect(pVScrollBar, &QScrollBar::rangeChanged,
            this, &ChatView::SlotVScrollBarMoved);
    // 自定义滚动条布局
    // 隐藏默认的垂直滚动条并使用水平布局来重新放置它
    QHBoxLayout *pHLayout = new QHBoxLayout();
    pHLayout->addWidget(pVScrollBar, 0, Qt::AlignRight);    // 将垂直滚动条添加到水平布局中，并右对齐
    pHLayout->setContentsMargins(QMargins(0, 0, 0, 0));
    m_pScrollArea->setLayout(pHLayout);
    pVScrollBar->setHidden(true);

    // 设置滚动区域的内容容器可调整大小
    m_pScrollArea->setWidgetResizable(true);
    // 为滚动区域安装事件过滤器，可能用于捕获和处理特定事件
    m_pScrollArea->installEventFilter(this);
    // initStyleSheet();
}

/**
 * @brief ChatView::AppendChatItem  向 ChatView 中追加一个新的聊天消息
 * @param item  新的聊天项
 */
void ChatView::AppendChatItem(QWidget *item)
{
    auto* pVLayout = qobject_cast<QVBoxLayout*>(m_pScrollArea->widget()->layout());
    qDebug() << "pVLayout->cout() is " << pVLayout->count();
    // 获取布局中倒数第二个位置的索引，最后一个子项为占位符（弹簧作用），即构造中 new 的那个 widget
    pVLayout->insertWidget(pVLayout->count()-1, item);
    m_bIsAppending = true;
}

/**
 * @brief ChatView::RemoveAllItem   清除聊天视图中的所有聊天项
 */
void ChatView::RemoveAllItem()
{
    // 获取垂直布局对象
    auto* pVLayout = qobject_cast<QVBoxLayout*>(m_pScrollArea->widget()->layout());

    // 获取布局中子项的数量
    int num_items = pVLayout->count();

    // 如果布局为空，直接返回
    if (num_items == 1) {
        return;
    }

    // 从后向前遍历并删除子项
    for (int i = num_items - 1; i >= 0; --i)
    {
        auto* item = pVLayout->takeAt(i);
        if (item)
        {
            if (QWidget* widget = item->widget())
            {
                delete widget;
            }
            delete item;
        }
    }
}

void ChatView::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this); // 根据 ChatView 的状态来初始化样式选项
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(event);
}

/**
 * @brief ChatView::eventFilter 根据鼠标的进入和离开事件，控制垂直滚动条的显示和隐藏
 * @param watched
 * @param event
 * @return
 */
bool ChatView::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_pScrollArea && event->type() == QEvent::Enter)
    {
        // 当鼠标进入时，根据滚动条的最大值决定是否显示滚动条（如果滚动条的最大值为 0（即不需要滚动），则隐藏滚动条）
        m_pScrollArea->verticalScrollBar()->setHidden(m_pScrollArea->verticalScrollBar()->maximum() == 0);
    }
    else if (watched == m_pScrollArea && event->type() == QEvent::Leave)
    {
        // 当鼠标离开时，隐藏滚动条
        m_pScrollArea->verticalScrollBar()->setHidden(true);
    }
    return QWidget::eventFilter(watched, event);
}

/**
 * @brief ChatView::SlotVScrollBarMoved     当有新消息时，自动将滚动条滚动到最底部
 */
void ChatView::SlotVScrollBarMoved()
{
    // 检查是否正在进行内容追加操作
    if (m_bIsAppending)
    {
        auto* pVScrollBar = m_pScrollArea->verticalScrollBar();
        // 将滚动条的位置设置为最大值（即滚动到最底部）
        pVScrollBar->setSliderPosition(pVScrollBar->maximum());
        // 添加 item可能调用多次
        QTimer::singleShot(500, [this]() {
            this->m_bIsAppending = false;
        });
    }
}
