#ifndef USERMGR_H
#define USERMGR_H
#include <QObject>
#include "singleton.h"
#include "userdata.h"

class UserMgr : public QObject, public Singleton<UserMgr>, public std::enable_shared_from_this<UserMgr>
{
    Q_OBJECT
    friend class Singleton<UserMgr>;
public:
    ~UserMgr() = default;

    int GetUID();
    QString GetName();
    QString GetIcon();
    std::shared_ptr<UserInfo> GetUserInfo();

    void SetToken(const QString& token);
    void SetUserInfo(std::shared_ptr<UserInfo> user_info);

    bool CheckFriendByUID(int uid);

    QMap<int, std::shared_ptr<ApplyInfo>> GetApplyList();
    bool AlreadyApply(int uid);
    void AddApplyList(std::shared_ptr<ApplyInfo> info);
    void AppendApplyList(QJsonArray array);

    std::shared_ptr<FriendInfo> GetFriendByUID(int uid);
    void AddFriend(std::shared_ptr<AuthRsp> auth_rsp);
    void AddFriend(std::shared_ptr<AuthInfo> auth_info);
    void AppendFriendList(QJsonArray array);

    std::vector<std::shared_ptr<FriendInfo>> GetChatListPerPage();
    bool IsLoadChatFin();
    void UpdateChatLoadedCount();
    std::vector<std::shared_ptr<FriendInfo>> GetConListPerPage();
    void UpdateContactLoadedCount();
    bool IsLoadConFin();

    void AppendFriendChatMsg(int friend_id, std::vector<std::shared_ptr<TextChatData>>);

private:
    UserMgr();

private:
    QString _token; // 客户端登录者被分配用于连接 ChatServer 的令牌
    std::shared_ptr<UserInfo> _user_info; // 客户端登录者的基本信息
    size_t _chat_loaded;       // 已经加载的历史聊天条目数量
    size_t _contact_loaded;    // 已经加载的联系人条目数量

    QMap<int, std::shared_ptr<ApplyInfo>> _apply_list;      // 好友申请列表，key是申请者的 UID
    std::vector<std::shared_ptr<FriendInfo>> _friend_list;  // 好友列表
    QMap<int, std::shared_ptr<FriendInfo>> _friend_map;     // 好友列表，key是好友的 UID
};

#endif // USERMGR_H
