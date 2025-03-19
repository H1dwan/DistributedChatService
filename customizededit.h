/******************************************************************************
 *
 * @file       customizededit.h
 * @brief      自定义的 QLineEdit 控件，能够实现限制输入的最大长度功能，同时重写了 focusOutEvent
 *             能够在失去焦点后执行相应的逻辑
 *
 * @author     Heesoo
 * @date       2025/01/07
 * @history
 *****************************************************************************/
#ifndef CUSTOMIZEDEDIT_H
#define CUSTOMIZEDEDIT_H

#include <QLineEdit>

class CustomizedEdit : public QLineEdit
{
    Q_OBJECT
public:
    CustomizedEdit(QWidget *parent = nullptr);
    void SetMaxLength(qsizetype max_len);

protected:
    void focusOutEvent(QFocusEvent * event) override;

private:
    void LimitTextLength(QString text);

private:
    qsizetype _max_len;

signals:
    void sig_focus_out();
};

#endif // CUSTOMIZEDEDIT_H
