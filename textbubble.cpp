#include "textbubble.h"
#include <QEvent>
#include <QTextBlock>
#include <QTextDocument>

TextBubble::TextBubble(ChatRole role, const QString& text, QWidget* parent)
    : BubbleFrame(role, parent)
{
    // 初始化 QTextEdit
    m_pTextEdit = new QTextEdit();
    m_pTextEdit->setReadOnly(true);
    m_pTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pTextEdit->setFont(QFont("Microsoft YaHei", 12));
    m_pTextEdit->installEventFilter(this);

    // 设置文本内容
    this->SetPlainText(text);
    // 将 m_pTextEdit 设置为气泡的内容控件
    BubbleFrame::SetWidget(m_pTextEdit);
    this->InitStyleSheet();
}

/**
 * @brief TextBubble::eventFilter   当 m_pTextEdit 触发绘制事件时，调用 AdjustTextHeight 方法调整文本高度
 * @param watched
 * @param event
 * @return
 */
bool TextBubble::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_pTextEdit && event->type() == QEvent::Paint)
    {
        this->AdjustTextHeight();
        return true;
    }
    return BubbleFrame::eventFilter(watched, event);
}

/**
 * @brief TextBubble::AdjustTextHeight  通过遍历文本块计算文本高度，并根据边距动态调整文本气泡框的高度
 */
void TextBubble::AdjustTextHeight()
{
    // 获取文档边距，即文本与边框之间的距离，doc_margin 是单边的边距值，因此总边距需要乘以 2
    qreal doc_margin = m_pTextEdit->document()->documentMargin();   // 字体到边框的距离默认为 4

    // 获取 QTextEdit 的文档对象（QTextDocument），用于遍历文本内容
    QTextDocument* doc = m_pTextEdit->document();

    // 获取文档的总高度
    qreal text_height = 0;
    // qreal text_height = doc->size().height();

    //把每一段的高度相加 =文本高
    for (auto it = doc->begin(); it != doc->end(); it = it.next())
    {
        QTextLayout *pLayout = it.layout();
        QRectF text_rect = pLayout->boundingRect(); //这段的rect
        text_height += text_rect.height();
    }

    // 获取布局边距
    int margin = this->layout()->contentsMargins().top();

    // 调整气泡框高度，文本高度 + 文档边距（上下） + 布局边距（上下）即 TextEdit 边框到气泡边框
    setFixedHeight(text_height + doc_margin * 2 + margin * 2);
}

/**
 * @brief TextBubble::SetPlainText  用于设置文本内容并动态调整气泡框的最大宽度
 * @param text
 */
void TextBubble::SetPlainText(const QString &text)
{
    // 设置文本内容
    m_pTextEdit->setPlainText(text);

    // 获取文档边距和布局边距
    qreal doc_margin = m_pTextEdit->document()->documentMargin();
    int margin_left = this->layout()->contentsMargins().left();
    int margin_right = this->layout()->contentsMargins().right();

    // 获取字体度量
    QFontMetricsF fm(m_pTextEdit->font());
    // 获取文档对象
    QTextDocument *doc = m_pTextEdit->document();
    // 遍历每一段，找到最宽段落的宽度
    int max_width = 0;
    for (QTextBlock it = doc->begin(); it != doc->end(); it = it.next())    // 字体总长
    {
        int txtW = int(fm.horizontalAdvance(it.text()));
        max_width = max_width < txtW ? txtW : max_width;    // 找到最长的那段
    }

    // 设置这个气泡的最大宽度 只需要设置一次
    int newWidth = max_width + doc_margin * 2 + (margin_left + margin_right);
    setMaximumWidth(qMin(newWidth, 500));

    // 调试信息
    qDebug() << "----------";
    qDebug() << "Max text width:" << max_width;
    qDebug() << "Document margin:" << doc_margin;
    qDebug() << "Layout margins (left, right):" << margin_left << margin_right;
    qDebug() << "New maximum width:" << newWidth;
}

void TextBubble::InitStyleSheet()
{
    m_pTextEdit->setStyleSheet("QTextEdit{background:transparent;border:none}");
}
