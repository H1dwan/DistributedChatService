#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include "userdata.h"

enum class ChatUIMode;
class StateWidget;

namespace Ui {
class ChatDialog;
}

class ChatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatDialog(QWidget *parent = nullptr);
    ~ChatDialog();
    // TODO: 测试用代码
    void _addChatUserList();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void _ShowSearchPage(bool b_show = true);
    void _AddLBGroup(StateWidget* lb);
    void _ClearLabelState(StateWidget* lb);

    void _HandleGlobalMousePress(QMouseEvent *event);

    void SetSelectChatItem(int uid = 0);
    void SetSelectChatPage(int uid = 0);

    void UpdateChatMsg(std::vector<std::shared_ptr<TextChatData>> msgdata);

    void _LoadMoreChatUser();
    void _LoadMoreConUser();

private:
    Ui::ChatDialog *ui;
    bool _b_loading;
    ChatUIMode _mode;   // sidebar中切换不同的模式
    ChatUIMode _state;
    int _cur_chat_uid;  // 当前聊天用户的 UID
    QWidget* _last_widget;
    QMap<ChatUIMode, QWidget*> _last_page;  // 不同模式下的上次浏览页面

    QList<StateWidget*> _lb_list;    // sidebar中的所有图标（消息、联系人等）
    QMap<int, QListWidgetItem*> _chat_items_added;  // 历史聊天列表中的聊天条目，key 是 UID

private slots:
    void slot_loading_chat_users();
    void slot_loading_contact_users();
    void slot_side_chat();
    void slot_side_contact();
    void slot_text_changed(const QString& str);
    void slot_item_clicked(QListWidgetItem *item);

    void slot_apply_friend(std::shared_ptr<AddFriendApply> apply);
    void slot_add_auth_friend(std::shared_ptr<AuthInfo>);
    void slot_auth_rsp(std::shared_ptr<AuthRsp>);
    void slot_jump_chat_item(std::shared_ptr<SearchInfo> si);
    void slot_friend_info_page(std::shared_ptr<UserInfo> user_info);
    void slot_switch_apply_friend_page();
    void slot_jump_chat_page_from_friend_info_page(std::shared_ptr<FriendInfo> friend_info);
    void slot_text_chat_msg(std::shared_ptr<TextChatMsg> msg);
    void slot_append_send_chat_msg(std::shared_ptr<TextChatData> msg);
};


enum class ChatUIMode
{
    SearchMode,     // 搜索模式
    ChatMode,       // 聊天模式
    ContactMode,    // 联系模式
};

#endif // CHATDIALOG_H
