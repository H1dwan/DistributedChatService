#include "customizededit.h"

CustomizedEdit::CustomizedEdit(QWidget *parent) : QLineEdit(parent), _max_len(0)
{
    // 连接搜索框文字长度限制
    connect(this, &QLineEdit::textChanged, this, &CustomizedEdit::LimitTextLength);
}

void CustomizedEdit::SetMaxLength(qsizetype max_len)
{
    _max_len = max_len;
}

void CustomizedEdit::focusOutEvent(QFocusEvent *event)
{
    // 执行失去焦点时的处理逻辑
    qDebug() << "CustomizedEdit focus out";
    // 调用基类的 focusOutEvent()方法，保证基类的行为得到执行
    QLineEdit::focusOutEvent(event);
    // 发送失去焦点得信号
    emit sig_focus_out();
}

void CustomizedEdit::LimitTextLength(QString text)
{
    if (_max_len <= 0)
    {
        return;
    }
    // 转换为字节数组（因为一个汉字可能为两个字节或三个字节）
    QByteArray bytes_array = text.toUtf8();
    if (bytes_array.size() > _max_len)
    {
        bytes_array = bytes_array.left(_max_len);
        // 还原为字符串
        this->setText(QString::fromUtf8(bytes_array));
    }
}
