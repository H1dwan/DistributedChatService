/******************************************************************************
 *
 * @file       friendinfoitem.h
 * @brief      联系人详情页中，战胜联系人信息的条目，例如备注、标签等
 *
 * @author     Heesoo
 * @date       2025/01/16
 * @history
 *****************************************************************************/
#ifndef FRIENDINFOITEM_H
#define FRIENDINFOITEM_H

#include <QWidget>

namespace Ui {
class FriendInfoItem;
}

class FriendInfoItem : public QWidget
{
    Q_OBJECT

public:
    explicit FriendInfoItem(QWidget *parent = nullptr);
    ~FriendInfoItem();

    void SetBackName(QString back_name);

private:
    Ui::FriendInfoItem *ui;
};

#endif // FRIENDINFOITEM_H
