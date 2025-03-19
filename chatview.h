/******************************************************************************
 *
 * @file       chatview.h
 * @brief      ChatView类实现了一个聊天视图，其中包含一个滚动区域来显示聊天内容。
 *
 * @author     Heesoo
 * @date       2025/01/04
 * @history
 *****************************************************************************/
#ifndef CHATVIEW_H
#define CHATVIEW_H
#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>

class ChatView : public QWidget
{
    Q_OBJECT
public:
    ChatView(QWidget* parent = nullptr);

    void AppendChatItem(QWidget* item);     // 尾插
    // void PrependChatItem(QWidget* item);    // 头插
    // void InsertChatItem(QWidget* before, QWidget* item);
    void RemoveAllItem();

protected:
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    // QVBoxLayout* m_pVLayout;
    QScrollArea* m_pScrollArea;  // 滚动区域
    bool m_bIsAppending;        // 用于控制聊天消息的追加

private slots:
    void SlotVScrollBarMoved();
};

#endif // CHATVIEW_H
