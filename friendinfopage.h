/******************************************************************************
 *
 * @file       friendinfopage.h
 * @brief      好友（联系人）的详情信息页
 *
 * @author     Heesoo
 * @date       2025/01/15
 * @history
 *****************************************************************************/
#ifndef FRIENDINFOPAGE_H
#define FRIENDINFOPAGE_H

#include <QDialog>
#include "userdata.h"

namespace Ui {
class FriendInfoPage;
}

class FriendInfoPage : public QDialog
{
    Q_OBJECT

public:
    explicit FriendInfoPage(QWidget *parent = nullptr);
    ~FriendInfoPage();

    void SetInfo(std::shared_ptr<FriendInfo> friend_info);

private:
    Ui::FriendInfoPage *ui;
    std::shared_ptr<FriendInfo> _friend_info;

signals:
    void sig_jump_chat_page(std::shared_ptr<FriendInfo> friend_info);

private slots:
    void on_msg_chat_clicked();
};

#endif // FRIENDINFOPAGE_H
