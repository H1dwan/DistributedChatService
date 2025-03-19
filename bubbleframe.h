/******************************************************************************
 *
 * @file       bubbleframe.h
 * @brief      气泡框的实现，气泡框采用水平布局，其内含一个 widget 用于包含文字、语音、图片
 *             等聊天项，该类是作为基类使用的
 *
 * @author     Heesoo
 * @date       2025/01/06
 * @history
 *****************************************************************************/
#ifndef BUBBLEFRAME_H
#define BUBBLEFRAME_H

#include <QFrame>
#include <QHBoxLayout>
#include "global.h"

class BubbleFrame : public QFrame
{
    Q_OBJECT
public:
    BubbleFrame(ChatRole role, QWidget* parent = nullptr);
    void SetWidget(QWidget* w);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QHBoxLayout* m_pHLayout;
    ChatRole m_role;
    int m_nMargin;
};

#endif // BUBBLEFRAME_H
