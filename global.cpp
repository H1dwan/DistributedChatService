#include "global.h"

QString gate_url_prefix = "";

/**
 * @brief repolish 用来刷新qss
 */
std::function<void(QWidget*)> repolish = [](QWidget* w) {
    w->style()->unpolish(w);
    w->style()->polish(w);
    w->update();
};

/**
 * @brief xoString: 利用异或对密码进行简单加密（对一个字符串进行两次相同的异或还是该字符串）
 */
std::function<QString(QString)> xoString = [](QString str) {
    QString encoded_str = str;
    int len = str.length() % 255;
    // 对每个字符进行异或操作
    // 这里假设字符都是 ASCII码，因此直接转换为 QChar
    for (int i = 0; i < len; ++i) {
        encoded_str[i] = QChar(static_cast<ushort>(str[i].unicode()) ^ static_cast<ushort>(len));
    }
    return encoded_str;
};
