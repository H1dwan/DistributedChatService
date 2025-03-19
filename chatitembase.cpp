#include "chatitembase.h"
#include <QSpacerItem>
#include <QGridLayout>

ChatItemBase::ChatItemBase(ChatRole role, QWidget *parent)
    : QWidget{parent}, m_role{role}
{
    // 创建用户名 label
    m_pNameLabel = new QLabel();
    m_pNameLabel->setObjectName("chat_user_name");
    m_pNameLabel->setFont({"Microsoft YaHei", 9});
    m_pNameLabel->setFixedHeight(20);
    // 创建用户头像 label
    m_pIconLabel = new QLabel();
    m_pIconLabel->setScaledContents(true);
    m_pIconLabel->setFixedSize(42, 42);
    // 创建消息气泡
    m_pBubble = new QWidget();
    // 创建空白占位符（用于调整消息气泡的位置），水平方向为扩展策略，垂直方向为最小策略
    QSpacerItem* pSpacerForBubble = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    // 创建网格布局
    QGridLayout* pGLayout = new QGridLayout(this);
    pGLayout->setVerticalSpacing(3);
    pGLayout->setHorizontalSpacing(3);
    pGLayout->setContentsMargins({3,3,3,3});    // 设置布局的外边距为 3 像素

    // 根据不同角色调整对应的布局
    switch (m_role) {
    case ChatRole::Self:
        m_pNameLabel->setContentsMargins(0, 0, 8, 0);
        m_pNameLabel->setAlignment(Qt::AlignRight);
        pGLayout->addWidget(m_pNameLabel, 0, 1, 1, 1);
        pGLayout->addWidget(m_pIconLabel, 0, 2, 2, 1, Qt::AlignTop);
        pGLayout->addWidget(m_pBubble, 1, 1, 1, 1);
        pGLayout->addItem(pSpacerForBubble, 1, 0, 1, 1);
        pGLayout->setColumnStretch(0, 2);
        pGLayout->setColumnStretch(1, 3);
        break;
    case ChatRole::Other:
        m_pNameLabel->setContentsMargins(8, 0, 0, 0);
        m_pNameLabel->setAlignment(Qt::AlignLeft);
        pGLayout->addWidget(m_pNameLabel, 0, 1, 1, 1);
        pGLayout->addWidget(m_pIconLabel, 0, 0, 2, 1, Qt::AlignTop);
        pGLayout->addWidget(m_pBubble, 1, 1, 1, 1);
        pGLayout->addItem(pSpacerForBubble, 1, 2, 1, 1);
        pGLayout->setColumnStretch(1, 3);
        pGLayout->setColumnStretch(2, 2);
        break;
    default:
        break;
    }

    this->setLayout(pGLayout);
}

void ChatItemBase::SetUserName(const QString &name)
{
    m_pNameLabel->setText(name);
}

void ChatItemBase::SetUserIcon(const QPixmap &icon)
{
    m_pIconLabel->setPixmap(icon);
}

/**
 * @brief ChatItemBase::SetWidget   将聊天消息项中的气泡部件 m_pBubble 替换为 w，替换后新的 w 成为布局的一部分
 * @param w
 */
void ChatItemBase::SetWidget(QWidget *w)
{
    auto* pGLayout = (qobject_cast<QGridLayout*>)(this->layout());
    // 替换后，w 会继承 m_pBubble 在布局中的位置和大小策略
    pGLayout->replaceWidget(m_pBubble, w);
    delete m_pBubble;   // 释放内存
    m_pBubble = w;
    // 强制更新布局
    pGLayout->update();
}
