#ifndef CHATITEMBASE_H
#define CHATITEMBASE_H

#include <QWidget>
#include <QLabel>
#include "global.h"

class ChatItemBase : public QWidget
{
    Q_OBJECT
public:
    ChatItemBase(ChatRole role, QWidget *parent = nullptr);
    void SetUserName(const QString& name);
    void SetUserIcon(const QPixmap& icon);
    void SetWidget(QWidget* w);

private:
    ChatRole m_role;        // 聊天角色（sender or receiver）
    QLabel* m_pNameLabel;   // 用户的昵称
    QLabel* m_pIconLabel;   // 用户的头像
    QWidget* m_pBubble;     // 聊天气泡框

signals:
};

#endif // CHATITEMBASE_H
