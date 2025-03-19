#include "bubbleframe.h"
#include <QPainter>

const int WIDTH_TRIANGLE = 8; // 气泡中三角形指示符的宽度

BubbleFrame::BubbleFrame(ChatRole role, QWidget *parent)
    : QFrame(parent), m_role(role), m_nMargin(3)
{
    m_pHLayout = new QHBoxLayout();
    if (m_role == ChatRole::Self)
    {
        // 右侧外边距增加 WIDTH_TRIANGLE，以便为三角形指示符留出空间
        m_pHLayout->setContentsMargins(m_nMargin, m_nMargin, m_nMargin + WIDTH_TRIANGLE, m_nMargin);
    }
    else if (m_role == ChatRole::Other)
    {
        // 左侧外边距增加 WIDTH_TRIANGLE，以便为三角形指示符留出空间
        m_pHLayout->setContentsMargins(m_nMargin + WIDTH_TRIANGLE, m_nMargin, m_nMargin, m_nMargin);
    }
    this->setLayout(m_pHLayout);
}

/**
 * @brief BubbleFrame::SetWidget    向气泡框中添加聊天项
 * @param w     聊天项，可以是文字、语音、图片等；
 */
void BubbleFrame::SetWidget(QWidget *w)
{
    // 如果气泡中已经有子部件，则直接返回
    if (m_pHLayout->count() == 0) {
        m_pHLayout->addWidget(w);
        this->update();
    }
}

void BubbleFrame::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setPen(Qt::NoPen);  // 设置画笔为无边框，即不绘制边框
    painter.setRenderHint(QPainter::Antialiasing, true); // 启用抗锯齿

    if (m_role == ChatRole::Self)
    {
        QColor bf_color(158, 234, 106);
        painter.setBrush(QBrush(bf_color));
        // 绘制圆角矩形气泡
        QRect bf_rect = QRect(0, 0, this->width() - WIDTH_TRIANGLE, this->height());
        painter.drawRoundedRect(bf_rect, 5, 5); // 使用 drawRoundedRect 绘制圆角矩形，圆角半径为 5 像素
        // 绘制三角形指示符
        QPointF points[3] = {
            QPointF(bf_rect.x() + bf_rect.width(), 12), // 左上顶点
            QPointF(bf_rect.x() + bf_rect.width(), 12 + WIDTH_TRIANGLE + 2), // 左下顶点
            QPointF(bf_rect.x() + bf_rect.width() + WIDTH_TRIANGLE, 10 + WIDTH_TRIANGLE - WIDTH_TRIANGLE / 2)  // 右上顶点
        };
        painter.drawPolygon(points, 3);
    }
    else if (m_role == ChatRole::Other)
    {
        QColor bf_color(Qt::white);
        painter.setBrush(QBrush(bf_color));
        // 绘制圆角矩形气泡
        QRect bf_rect = QRect(WIDTH_TRIANGLE, 0, this->width() - WIDTH_TRIANGLE, this->height());
        painter.drawRoundedRect(bf_rect, 5, 5);
        // 绘制三角形指示符
        QPointF points[3] = {
            QPointF(bf_rect.x(), 12), // 左上顶点
            QPointF(bf_rect.x(), 10 + WIDTH_TRIANGLE + 2), // 左下顶点
            QPointF(bf_rect.x() - WIDTH_TRIANGLE, 10 + WIDTH_TRIANGLE - WIDTH_TRIANGLE / 2) // 右上顶点
        };
        painter.drawPolygon(points, 3);
    }

    QFrame::paintEvent(event);
}

