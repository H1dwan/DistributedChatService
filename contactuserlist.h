/******************************************************************************
 *
 * @file       contactuserlist.h
 * @brief      联系人列表
 *
 * @author     Heesoo
 * @date       2025/01/14
 * @history
 *****************************************************************************/
#ifndef CONTACTUSERLIST_H
#define CONTACTUSERLIST_H

#include <QListWidget>
#include "userdata.h"

class ContactUserItem;

class ContactUserList : public QListWidget
{
    Q_OBJECT
public:
    ContactUserList(QWidget *parent = nullptr);

    void ShowRedPoint(bool show = true);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void AddContactUserList();

private:
    bool _load_pending;
    ContactUserItem* _add_friend_item;  // 添加好友的条目
    QListWidgetItem* _groupitem;        // 分组的条目（如新的朋友、联系人）用于提示、分隔

signals:
    void sig_loading_contact_users();
    void sig_switch_apply_friend_page();
    void sig_switch_friend_info_page(std::shared_ptr<UserInfo> user_info);

public slots:
    void slot_item_clicked(QListWidgetItem *item);
    void slot_add_auth_firend(std::shared_ptr<AuthInfo>);
    void slot_auth_rsp(std::shared_ptr<AuthRsp>);
};

#endif // CONTACTUSERLIST_H
