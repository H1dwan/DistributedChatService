#ifndef TEXTBUBBLE_H
#define TEXTBUBBLE_H

#include <QTextEdit>
#include "bubbleframe.h"

class TextBubble final : public BubbleFrame
{
    Q_OBJECT
public:
    TextBubble(ChatRole role, const QString& text, QWidget* parent = nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void AdjustTextHeight();
    void SetPlainText(const QString& text);
    void InitStyleSheet();

private:
    QTextEdit* m_pTextEdit;
};

#endif // TEXTBUBBLE_H
