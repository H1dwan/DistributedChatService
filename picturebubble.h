/******************************************************************************
 *
 * @file       picturebubble.h
 * @brief      图片气泡框的实现
 *
 * @author     Heesoo
 * @date       2025/01/06
 * @history
 *****************************************************************************/
#ifndef PICTUREBUBBLE_H
#define PICTUREBUBBLE_H

#include "bubbleframe.h"

class PictureBubble final : public BubbleFrame
{
    Q_OBJECT
public:
    PictureBubble(ChatRole role, const QPixmap& picture, QWidget* parent = nullptr);
};

#endif // PICTUREBUBBLE_H
