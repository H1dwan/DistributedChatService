/******************************************************************************
 *
 * @file       chatuserwid.h
 * @brief      聊天模式下，聊天列表中与用户聊天的条目
 *
 * @author     Heesoo
 * @date       2025/01/14
 * @history
 *****************************************************************************/
#ifndef CHATUSERWID_H
#define CHATUSERWID_H
#include "listitembase.h"
#include "userdata.h"

namespace Ui {
class ChatUserWid;
}

class ChatUserWid : public ListItemBase
{
    Q_OBJECT

public:
    explicit ChatUserWid(QWidget *parent = nullptr);
    ~ChatUserWid();

    QSize sizeHint() const override {
        return QSize(250, 70);  // 返回自定义尺寸
    }

    void SetInfo(QString head_addr, QString name, QString msg);
    void SetInfo(std::shared_ptr<UserInfo>user_info);
    void SetInfo(std::shared_ptr<FriendInfo> friend_info);
    std::shared_ptr<UserInfo> GetUserInfo();

    void ShowRedPoint(bool show=true);
    void UpdateLastMsg(std::vector<std::shared_ptr<TextChatData>> msgs);

private:
    Ui::ChatUserWid *ui;
    std::shared_ptr<UserInfo> _user_info;
};

#endif // CHATUSERWID_H
