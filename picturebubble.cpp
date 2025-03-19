#include "picturebubble.h"
#include <QLabel>

const int PIC_MAX_WIDTH = 160;
const int PIC_MAX_HEIGHT = 90;

PictureBubble::PictureBubble(ChatRole role, const QPixmap& picture, QWidget* parent)
    :BubbleFrame(role, parent)
{
    QLabel* pLabel = new QLabel();
    pLabel->setScaledContents(true);
    QPixmap pic = picture.scaled(QSize(PIC_MAX_WIDTH, PIC_MAX_HEIGHT), Qt::KeepAspectRatio);
    pic.setDevicePixelRatio(devicePixelRatio()); // 支持高 DPI
    pLabel->setPixmap(pic);
    // 调用基类的 SetWidget 方法，将 pLabel 设置为气泡框的内容控件
    BubbleFrame::SetWidget(pLabel);
    // 获取布局的左右边距和垂直边距（上下边距相同）
    int left_margin = this->layout()->contentsMargins().left();
    int right_margin = this->layout()->contentsMargins().right();
    int v_margin = this->layout()->contentsMargins().bottom();
    // 调整气泡框的大小，气泡框的总宽度：图片宽度 + 左右边距，气泡框的总高度：图片高度 + 上下边距
    setFixedSize(pic.width() + left_margin + right_margin, pic.height() + v_margin * 2);
}
